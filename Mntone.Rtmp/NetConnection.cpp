#include "pch.h"
#include "NetConnection.h"
#include "NetStream.h"
#include "Handshaker.h"
#include "RtmpHelper.h"
#include "Command/NetConnectionConnectCommand.h"

using namespace Mntone::Rtmp;

const auto DEFAULT_WINDOW_SIZE = 2500000;
const auto DEFAULT_CHUNK_SIZE = 128;
const auto DEFAULT_BUFFER_MILLSECONDS = 5000;

NetConnection::NetConnection()
	: DefaultEncodingType_( Mntone::Data::Amf::AmfEncodingType::Amf3 )
	, connection_( ref new Connection() )
	, _latestTransactionId( 2 )
	, rxHeaderBuffer_( 11 )
	, rxWindowSize_( DEFAULT_WINDOW_SIZE ), txWindowSize_( DEFAULT_WINDOW_SIZE )
	, rxChunkSize_( DEFAULT_CHUNK_SIZE ), txChunkSize_( DEFAULT_CHUNK_SIZE )
{ }

NetConnection::~NetConnection()
{
	delete connection_;
	Closed( this, ref new NetConnectionClosedEventArgs() );
}

Windows::Foundation::IAsyncAction^ NetConnection::ConnectAsync( Windows::Foundation::Uri^ uri )
{
	return ConnectAsync( ref new RtmpUri( uri ) );
}

Windows::Foundation::IAsyncAction^ NetConnection::ConnectAsync( Windows::Foundation::Uri^ uri, Command::IRtmpCommand^ connectCommand )
{
	if( connectCommand->Type != "connect" )
		throw ref new Platform::InvalidArgumentException();

	return ConnectAsync( ref new RtmpUri( uri ), connectCommand );
}

Windows::Foundation::IAsyncAction^ NetConnection::ConnectAsync( RtmpUri^ uri )
{
	auto connect = ref new Command::NetConnectionConnectCommand( uri->App );
	connect->TcUrl = uri->ToString();
	return ConnectAsync( uri, connect );
}

Windows::Foundation::IAsyncAction^ NetConnection::ConnectAsync( RtmpUri^ uri, Command::IRtmpCommand^ connectCommand )
{
	return create_async( [=]
	{
		startTime_ = GetWindowsTime();
		Uri_ = uri;

		auto task = connection_->ConnectAsync( Uri_->Host, Uri_->Port.ToString() );
		return task.then( [=]
		{
			Handshaker::Handshake( this );
			SendActionAsync( 0, connectCommand->Commandify() );
			create_task( [=] { Receive(); } );
		} );
	} );
}

void NetConnection::AttachNetStream( NetStream^ stream )
{
	using namespace Mntone::Data::Amf;

	stream->parent_ = this;

	const auto tid = _latestTransactionId++;
	netStreamTemporary_.emplace( tid, stream );

	auto cmd = ref new AmfArray();
	cmd->Append( AmfValue::CreateStringValue( "createStream" ) );				// Command name
	cmd->Append( AmfValue::CreateNumberValue( static_cast<float64>( tid ) ) );	// Transaction id
	cmd->Append( ref new AmfValue() );											// Command object: set to null type when do not exist
	SendActionAsync( cmd );
}

void NetConnection::UnattachNetStream( NetStream^ stream )
{
	using namespace Mntone::Data::Amf;

	bindingNetStream_.erase( stream->streamId_ );
}

#pragma region Network operation (Server to Client)

void NetConnection::Receive()
{
	while( true )
		ReceiveImpl();
}

void NetConnection::ReceiveImpl()
{
	const auto& p = rxHeaderBuffer_.data();

	// ---[ Read and check header ]----------
	// Copes with Stack Overflow
	while( connection_->TryRead( p, 1 ) == 0 );

	const auto formatType = ( p[0] >> 6 ) & 0x03;
	uint16 chunkStreamId = p[0] & 0x3f;

	if( chunkStreamId == 0 )
	{
		connection_->Read( p, 1 );
		chunkStreamId = p[0] + 64;
	}
	else if( chunkStreamId == 1 )
	{
		connection_->Read( p, 2 );
		ConvertBigEndian( p, &chunkStreamId, 2 );
		chunkStreamId += 64;
	}

	// ---[ Get object ]----------
	std::shared_ptr<rtmp_packet> packet;
	auto ret = rxBakPackets_.find( chunkStreamId );
	if( ret == rxBakPackets_.end() )
	{
		packet = std::make_shared<rtmp_packet>( );
		packet->ChunkStreamId = chunkStreamId;
		packet->Length = 0;
		rxBakPackets_.emplace( chunkStreamId, packet );
	}
	else
		packet = ret->second;

	// ---[ Read header body ]----------
	switch( formatType )
	{
	case 0:
		connection_->Read( p, 11 );

		packet->Timestamp = 0; // initialize
		ConvertBigEndian( p, &packet->Timestamp, 3 );
		ConvertBigEndian( p + 3, &packet->Length, 3 );
		packet->TypeId = static_cast<type_id_type>( p[6] );
		ConvertLittleEndian( p + 7, &packet->StreamId, 4 ); // LE

		if( packet->Timestamp == 0xffffff )
		{
			connection_->Read( p, 4 );
			ConvertBigEndian( p, &packet->Timestamp, 4 );
		}
		packet->TimestampDelta = packet->Timestamp;
		break;

	case 1:
		{
			connection_->Read( p, 7 );

			packet->TimestampDelta = 0;
			ConvertBigEndian( p, &packet->TimestampDelta, 3 );
			ConvertBigEndian( p + 3, &packet->Length, 3 );
			packet->TypeId = static_cast<type_id_type>( p[6] );

			if( packet->TimestampDelta == 0xffffff )
			{
				connection_->Read( p, 4 );
				ConvertBigEndian( p, &packet->TimestampDelta, 4 );
			}
			packet->Timestamp += packet->TimestampDelta;
			break;
		}

	case 2:
		{
			connection_->Read( p, 3 );

			packet->TimestampDelta = 0;
			ConvertBigEndian( p, &packet->TimestampDelta, 3 );

			if( packet->TimestampDelta == 0xffffff )
			{
				connection_->Read( p, 4 );
				ConvertBigEndian( p, &packet->TimestampDelta, 4 );
			}
			packet->Timestamp += packet->TimestampDelta;
			break;
		}

	case 3:
		{
			if( packet->TimestampDelta > 0xffffff )
			{
				connection_->Read( p, 4 );
				ConvertBigEndian( p, &packet->TimestampDelta, 4 );
			}
			packet->Timestamp += packet->TimestampDelta;
			break;
		}
	}

	// ---[ Read message body ]----------
	if( packet->Length == 0 )
		return;

	const auto& length = packet->Length;
	std::vector<uint8> data( length );
	{
		auto dp = data.data();
		for( auto i = 0u; i < length; i += rxChunkSize_, dp += rxChunkSize_ )
		{
			if( i + rxChunkSize_ < length )
				connection_->Read( dp, rxChunkSize_ + 1 );
			else
				connection_->Read( dp, length - i );
		}
	}

	// ---[ Callback ]----------
	const auto& sid = packet->StreamId;
	if( sid == 0 )
		OnMessage( *packet.get(), std::move( data ) );
	else
	{
		const auto ret = bindingNetStream_.find( sid );
		if( ret != bindingNetStream_.end() )
			ret->second->OnMessage( *packet.get(), std::move( data ) );
	}
}

void NetConnection::OnMessage( const rtmp_packet packet, std::vector<uint8> data )
{
	if( packet.ChunkStreamId == 2 )
	{
		OnNetworkMessage( std::move( packet ), std::move( data ) );
		return;
	}

	switch( packet.TypeId )
	{
	case type_id_type::tid_command_message_amf0: OnCommandMessageAmf0( std::move( packet ), std::move( data ) ); break;
	case type_id_type::tid_command_message_amf3: OnCommandMessageAmf3( std::move( packet ), std::move( data ) ); break;
	}
}

void NetConnection::OnNetworkMessage( const rtmp_packet packet, std::vector<uint8> data )
{
	switch( packet.TypeId )
	{
	case type_id_type::tid_set_chunk_size: ConvertBigEndian( data.data(), &rxChunkSize_, 4 ); break;
	case type_id_type::tid_abort_message: break;
	case type_id_type::tid_acknowledgement: break;
	case type_id_type::tid_user_control_message: OnUserControlMessage( std::move( packet ), std::move( data ) ); break;
	case type_id_type::tid_window_acknowledgement_size: ConvertBigEndian( data.data(), &rxWindowSize_, 4 ); break;
	case type_id_type::tid_set_peer_bandwidth:
		{
			uint32 buf;
			ConvertBigEndian( data.data(), &buf, 4 );
			//const auto limitType = static_cast<limit_type>( data[4] );
			WindowAcknowledgementSizeAsync( buf );
			break;
		}
	}
}

void NetConnection::OnUserControlMessage( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	uint16 buf;
	ConvertBigEndian( data.data(), &buf, 2 );

	const auto& messageType = static_cast<UserControlMessageEventType>( buf );
	switch( messageType )
	{
	case UserControlMessageEventType::StreamBegin:
		{
			uint32 streamId;
			ConvertBigEndian( data.data() + 2, &streamId, 4 );
			if( streamId == 0 )
				SetBufferLengthAsync( 0, DEFAULT_BUFFER_MILLSECONDS );
			break;
		}

	case UserControlMessageEventType::StreamEof:
	case UserControlMessageEventType::StreamDry:
	case UserControlMessageEventType::StreamIsRecorded:
		{
			//uint32 streamId;
			//ConvertBigEndian( data.data() + 2, &streamId, 4 );
			break;
		}
	case UserControlMessageEventType::PingRequest:
		{
			uint32 timestamp;
			ConvertBigEndian( data.data() + 2, &timestamp, 4 );
			PingResponseAsync( timestamp );
			break;
		}
	}
}

void NetConnection::OnCommandMessageAmf0( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	OnCommandMessage( std::move( RtmpHelper::ParseAmf0( std::move( data ) ) ) );
}

void NetConnection::OnCommandMessageAmf3( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	Mntone::Data::Amf::AmfArray^ amf;
	if( DefaultEncodingType_ == Mntone::Data::Amf::AmfEncodingType::Amf3 )
	{ }
	else
		amf = RtmpHelper::ParseAmf0( std::move( data ) );
	OnCommandMessage( std::move( amf ) );
}

void NetConnection::OnCommandMessage( Mntone::Data::Amf::AmfArray^ amf )
{
	if( amf == nullptr ) return;
	const auto& name = amf->GetStringAt( 0 );
	const auto& tid = static_cast<uint32>( amf->GetNumberAt( 1 ) );
	
	// for connect result (tid = 1)
	if( tid == 1 )
	{
		//const auto& properties = amf->GetObjectAt( 2 );
		const auto& information = amf->GetObjectAt( 3 );

		//const auto fmsVer = properties->GetNamedString( "fmsVer" );
		//const auto capabilities = properties->GetNamedDouble( "capabilities" );
		//const auto level = information->GetNamedString( "level" );
		//const auto description = information->GetNamedString( "description" );
		//const auto objectEncoding = information->GetNamedDouble( "objectEncoding" );

		const auto& code = information->GetNamedString( "code" );
		const auto& nsc = RtmpHelper::ParseNetConnectionConnectCode( code->Data() );
		StatusUpdated( this, ref new NetStatusUpdatedEventArgs( nsc ) );
		return;
	}

	// for createStream result
	{
		const auto ret = netStreamTemporary_.find( tid );
		if( ret != netStreamTemporary_.end() )
		{
			if( name == "_result" )
			{
				//const auto commandBuf = amf->GetAt( 2 );
				//if( commandBuf->ValueType != Mntone::Data::Amf::AmfValueType::Object )
				//	const auto command = commandBuf->GetObject();
				const auto& sid = static_cast<uint32>( amf->GetNumberAt( 3 ) );

				auto stream = ret->second;
				stream->streamId_ = sid;
				bindingNetStream_.emplace( sid, stream );
				stream->AttachedImpl();
				SetBufferLengthAsync( sid, DEFAULT_BUFFER_MILLSECONDS );
			}
			netStreamTemporary_.erase( ret );
			return;
		}
	}

	// for call result (tid = 0 or choice)
	{
		//const auto commandBuf = amf->GetAt( 2 );
		//if( commandBuf->ValueType != Mntone::Data::Amf::AmfValueType::Object )
		//	const auto command = commandBuf->GetObject();
		//const auto response = amf->GetObject();
	}
}

#pragma endregion

#pragma region Network operation (Client to Server)

task<void> NetConnection::SetChunkSizeAsync( uint32 chunkSize )
{
	if( chunkSize > 0x7fffffff )
		throw ref new Platform::InvalidArgumentException( "Invalid chunkSize. Valid chunkSize is 1 to 2147483647." );

	txChunkSize_ = chunkSize;

	std::vector<uint8> buf( 4 );
	ConvertBigEndian( &chunkSize, buf.data(), 4 );
	return SendNetworkAsync( type_id_type::tid_set_chunk_size, std::move( buf ) );
}

task<void> NetConnection::AbortMessageAsync( uint32 chunkStreamId )
{
	std::vector<uint8> buf( 4 );
	ConvertBigEndian( &chunkStreamId, buf.data(), 4 );
	return SendNetworkAsync( type_id_type::tid_abort_message, std::move( buf ) );
}

task<void> NetConnection::AcknowledgementAsync( uint32 sequenceNumber )
{
	std::vector<uint8> buf( 4 );
	ConvertBigEndian( &sequenceNumber, buf.data(), 4 );
	return SendNetworkAsync( type_id_type::tid_acknowledgement, std::move( buf ) );
}

task<void> NetConnection::WindowAcknowledgementSizeAsync( uint32 acknowledgementWindowSize )
{
	txWindowSize_ = acknowledgementWindowSize;

	std::vector<uint8> buf( 4 );
	ConvertBigEndian( &acknowledgementWindowSize, buf.data(), 4 );
	return SendNetworkAsync( type_id_type::tid_window_acknowledgement_size, std::move( buf ) );
}

task<void> NetConnection::SetPeerBandWidthAsync( uint32 windowSize, limit_type type )
{
	std::vector<uint8> buf( 5 );
	ConvertBigEndian( &windowSize, buf.data(), 4 );
	buf[4] = type;
	return SendNetworkAsync( type_id_type::tid_set_peer_bandwidth, std::move( buf ) );
}

task<void> NetConnection::SetBufferLengthAsync( const uint32 streamId, const uint32 bufferLength )
{
	std::vector<uint8> buf( 10 );
	ConvertBigEndian( &streamId, buf.data() + 2, 4 );
	ConvertBigEndian( &bufferLength, buf.data() + 6, 4 );
	return UserControlMessageEventAsync( UserControlMessageEventType::SetBufferLength, std::move( buf ) );
}

task<void> NetConnection::PingResponseAsync( const uint32 timestamp )
{
	std::vector<uint8> buf( 6 );
	ConvertBigEndian( &timestamp, buf.data() + 2, 4 );
	return UserControlMessageEventAsync( UserControlMessageEventType::PingResponse, std::move( buf ) );
}

task<void> NetConnection::UserControlMessageEventAsync( UserControlMessageEventType type, std::vector<uint8> data )
{
	const uint16 cType = static_cast<uint16>( type );
	ConvertBigEndian( &cType, data.data(), 2 );
	return SendNetworkAsync( type_id_type::tid_user_control_message, std::move( data ) );
}

task<void> NetConnection::SendNetworkAsync( const type_id_type type, const std::vector<uint8> data )
{
	rtmp_packet packet;
	packet.ChunkStreamId = 2; // for Network (2)
	packet.Timestamp = GetWindowsTime() - startTime_;
	packet.Length = static_cast<uint32>( data.size() );
	packet.TypeId = type;
	packet.StreamId = 0;

	return SendAsync( std::move( packet ), std::move( data ), true );
}

task<void> NetConnection::SendActionAsync( Mntone::Data::Amf::AmfArray^ amf )
{
	return SendActionAsync( 0, amf );
}

task<void> NetConnection::SendActionAsync( uint32 streamId, Mntone::Data::Amf::AmfArray^ amf )
{
	auto amfData = amf->Sequencify( Mntone::Data::Amf::AmfEncodingType::Amf0 );
	const auto& length = amfData->Length - 5;

	rtmp_packet packet;
	packet.ChunkStreamId = 3; // for Action (3)
	packet.Timestamp = GetWindowsTime() - startTime_;
	packet.Length = length;
	packet.TypeId = type_id_type::tid_command_message_amf0;
	packet.StreamId = streamId;

	std::vector<uint8> buf( length );
	memcpy( buf.data(), amfData->Data + 5, length );
	return SendAsync( std::move( packet ), std::move( buf ) );
}

task<void> NetConnection::SendAsync( const rtmp_packet packet, const std::vector<uint8> data, const bool isFormatTypeZero )
{
	return create_task( [=] { SendImpl( packet, std::move( data ), isFormatTypeZero ); } );
}

void NetConnection::SendImpl( rtmp_packet packet, const std::vector<uint8> data, const bool isFormatTypeZero )
{
	auto inPtr = data.data();
	const auto inLen = packet.Length;
	const auto chunkSize = txChunkSize_;

	// ---[ Header ]----------
	const auto header = CreateHeader( std::move( packet ), isFormatTypeZero );
	const auto headerLength = static_cast<uint32>( header.size() );

	std::vector<uint8> buf( headerLength + inLen + ( inLen - 1 ) / chunkSize );
	memcpy( buf.data(), header.data(), headerLength );

	auto outPtr = buf.data() + headerLength;

	// ---[ Data ]----------
	bool odd = true;
	for( auto i = 0u; i < inLen - inLen % chunkSize; i += chunkSize, inPtr += chunkSize, odd != odd )
	{
		memcpy( outPtr, inPtr, chunkSize );
		outPtr += chunkSize;
		*( outPtr++ ) = odd ? 0xc3 : 0xc4;
	}
	memcpy( outPtr, inPtr, inLen % chunkSize );

	// ---[ Send ]----------
	connection_->Write( buf );
}

std::vector<uint8> NetConnection::CreateHeader( rtmp_packet packet, bool isFormatTypeZero )
{
	std::vector<uint8> data( 18 );
	size_t length( 0 );
	auto ptr = data.data();

	// ---[ Get object ]----------
	std::shared_ptr<rtmp_packet> bakPacket;
	auto ret = txBakPackets_.find( packet.ChunkStreamId );
	if( ret == txBakPackets_.end() )
		isFormatTypeZero = true;
	else
		bakPacket = ret->second;

	// ---[ Decide formatType ]----------
	uint8 formatType;
	if( isFormatTypeZero )
		formatType = 0;
	else if( packet.StreamId == bakPacket->StreamId )
	{
		if( packet.TypeId == bakPacket->TypeId && packet.Length == bakPacket->Length )
		{
			if( packet.Timestamp == bakPacket->Timestamp + 2 * bakPacket->TimestampDelta )
			{
				formatType = 3;
				packet.TimestampDelta = bakPacket->TimestampDelta;
			}
			else if( packet.Timestamp < bakPacket->Timestamp )
				formatType = 0;
			else
				formatType = 2;
		}
		else
			formatType = 1;
	}
	else
		formatType = 0;

	// ---[ Chunk basic header ]----------
	ptr[0] = formatType << 6;
	if( packet.ChunkStreamId < 64 )
	{
		ptr[0] |= packet.ChunkStreamId; //& 0x3f;

		length += 1;
		ptr += 1;
	}
	else if( packet.ChunkStreamId < 320 )
	{
		ptr[1] = static_cast<uint8>( packet.ChunkStreamId - 64 );

		length += 2;
		ptr += 2;
	}
	else if( packet.ChunkStreamId < 65600 )
	{
		ptr[0] |= 1;
		uint16 buf = packet.ChunkStreamId - 64;
		ConvertBigEndian( &buf, ptr + 1, 2 );

		length += 3;
		ptr += 3;
	}
	else
		throw ref new Platform::InvalidArgumentException();

	// ---[ Chunk message header ]----------
	switch( formatType )
	{
	case 0:
		ConvertBigEndian( &packet.Length, ptr + 3, 3 );
		ptr[6] = static_cast<uint8>( packet.TypeId );
		ConvertLittleEndian( &packet.StreamId, ptr + 7, 4 ); // LE
		if( packet.Timestamp >= 0xffffff )
		{
			ptr[0] = ptr[1] = ptr[2] = 0xff;
			ConvertBigEndian( &packet.Timestamp, ptr + 11, 4 );
			length += 15;
		}
		else
		{
			ConvertBigEndian( &packet.Timestamp, ptr, 3 );
			length += 11;
		}
		break;
	case 1:
		{
			packet.TimestampDelta = packet.Timestamp - bakPacket->Timestamp;
			ConvertBigEndian( &packet.Length, ptr + 3, 3 );
			ptr[6] = static_cast<uint8>( packet.TypeId );
			if( packet.TimestampDelta >= 0xffffff )
			{
				ptr[0] = ptr[1] = ptr[2] = 0xff;
				ConvertBigEndian( &packet.TimestampDelta, ptr + 7, 4 );
				length += 11;
			}
			else
			{
				ConvertBigEndian( &packet.TimestampDelta, ptr, 3 );
				length += 7;
			}
			break;
		}
	case 2:
		{
			packet.TimestampDelta = packet.Timestamp - bakPacket->Timestamp;
			if( packet.TimestampDelta >= 0xffffff )
			{
				ptr[0] = ptr[1] = ptr[2] = 0xff;
				ConvertBigEndian( &packet.TimestampDelta, ptr + 3, 4 );
				length += 7;
			}
			else
			{
				ConvertBigEndian( &packet.TimestampDelta, ptr, 3 );
				length += 3;
			}
			break;
		}
	case 3:		
		if( packet.TimestampDelta >= 0xffffff )
		{
			ConvertBigEndian( &packet.TimestampDelta, ptr + 3, 4 );
			length += 4;
		}
		break;
	}

	data.resize( length );
	txBakPackets_.emplace( packet.ChunkStreamId, std::make_shared<rtmp_packet>( std::move( packet ) ) );
	return std::move( data );
}

#pragma endregion