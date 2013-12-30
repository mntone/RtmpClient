#include "pch.h"
#include "NetConnection.h"
#include "NetStream.h"
#include "Handshaker.h"
#include "RtmpHelper.h"
#include "Command/NetConnectionConnectCommand.h"

using namespace std;
using namespace Concurrency;
using namespace mntone::rtmp;
using namespace Mntone::Rtmp;

const auto DEFAULT_WINDOW_SIZE = 2500000;
const auto DEFAULT_CHUNK_SIZE = 128;
const auto DEFAULT_BUFFER_MILLSECONDS = 5000;

NetConnection::NetConnection()
	: connection_( ref new Connection() )
	, latestTransactionId_( 2 )
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
		startTime_ = utility::get_windows_time();
		Uri_ = uri;

		auto task = connection_->ConnectAsync( Uri_->Host, Uri_->Port.ToString() );
		return task.then( [=]
		{
			Handshaker::Handshake( this );
			SendActionAsync( 0, connectCommand->Commandify() );
			create_task( [=] { Receive(); }, task_continuation_context::use_arbitrary() );
		}, task_continuation_context::use_arbitrary() );
	} );
}

task<void> NetConnection::AttachNetStream( NetStream^ stream )
{
	using namespace Mntone::Data::Amf;

	stream->parent_ = this;

	const auto tid = latestTransactionId_++;
	netStreamTemporary_.emplace( tid, stream );

	auto cmd = ref new AmfArray();
	cmd->Append( AmfValue::CreateStringValue( "createStream" ) );				// Command name
	cmd->Append( AmfValue::CreateNumberValue( static_cast<float64>( tid ) ) );	// Transaction id
	cmd->Append( ref new AmfValue() );											// Command object: set to null type when do not exist
	return SendActionAsync( cmd );
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

	const uint8_t& formatType = ( p[0] >> 6 ) & 0x03;
	uint16 chunkStreamId = p[0] & 0x3f;

	if( chunkStreamId == 0 )
	{
		connection_->Read( p, 1 );
		chunkStreamId = p[0] + 64;
	}
	else if( chunkStreamId == 1 )
	{
		connection_->Read( p, 2 );
		utility::convert_big_endian( p, 2, &chunkStreamId );
		chunkStreamId += 64;
	}

	// ---[ Get object ]----------
	shared_ptr<rtmp_packet> packet;
	const auto& itr = rxBakPackets_.lower_bound( chunkStreamId );
	if( itr != rxBakPackets_.end() && itr->first == chunkStreamId )
	{
		packet = itr->second;
	}
	else
	{
		packet = make_shared<rtmp_packet>();
		packet->chunk_stream_id_ = chunkStreamId;
		packet->length_ = 0;
		rxBakPackets_.emplace_hint( itr, chunkStreamId, packet );
	}

	// ---[ Read header body ]----------
	switch( formatType )
	{
	case 0:
		connection_->Read( p, 11 );

		packet->timestamp_ = 0; // initialize
		utility::convert_big_endian( p, 3, &packet->timestamp_ );
		utility::convert_big_endian( p + 3, 3, &packet->length_ );
		packet->type_id_ = static_cast<type_id_type>( p[6] );
		utility::convert_little_endian( p + 7, 4, &packet->stream_id_ ); // LE

		if( packet->timestamp_ == 0xffffff )
		{
			connection_->Read( p, 4 );
			utility::convert_big_endian( p, 4, &packet->timestamp_ );
		}
		packet->timestamp_delta_ = packet->timestamp_;
		break;

	case 1:
		{
			connection_->Read( p, 7 );

			packet->timestamp_delta_ = 0;
			utility::convert_big_endian( p, 3, &packet->timestamp_delta_ );
			utility::convert_big_endian( p + 3, 3, &packet->length_ );
			packet->type_id_ = static_cast<type_id_type>( p[6] );

			if( packet->timestamp_delta_ == 0xffffff )
			{
				connection_->Read( p, 4 );
				utility::convert_big_endian( p, 4, &packet->timestamp_delta_ );
			}
			packet->timestamp_ += packet->timestamp_delta_;
			break;
		}

	case 2:
		{
			connection_->Read( p, 3 );

			packet->timestamp_delta_ = 0;
			utility::convert_big_endian( p, 3, &packet->timestamp_delta_ );

			if( packet->timestamp_delta_ == 0xffffff )
			{
				connection_->Read( p, 4 );
				utility::convert_big_endian( p, 4, &packet->timestamp_delta_ );
			}
			packet->timestamp_ += packet->timestamp_delta_;
			break;
		}

	case 3:
		{
			if( packet->timestamp_delta_ > 0xffffff )
			{
				connection_->Read( p, 4 );
				utility::convert_big_endian( p, 4, &packet->timestamp_delta_ );
			}
			packet->timestamp_ += packet->timestamp_delta_;
			break;
		}
	}

	// ---[ Read message body ]----------
	if( packet->length_ == 0 )
		return;

	const auto& length = packet->length_;
	vector<uint8> data( length );
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
	const auto& sid = packet->stream_id_;
	if( sid == 0 )
	{
		OnMessage( *packet.get(), move( data ) );
	}
	else
	{
		const auto& itr = bindingNetStream_.lower_bound( sid );
		if( itr != bindingNetStream_.cend() && itr->first == sid )
			itr->second->OnMessage( *packet.get(), move( data ) );
	}
}

void NetConnection::OnMessage( const rtmp_packet packet, vector<uint8> data )
{
	if( packet.chunk_stream_id_ == 2 )
	{
		OnNetworkMessage( move( packet ), move( data ) );
		return;
	}

	switch( packet.type_id_ )
	{
	case type_id_type::tid_command_message_amf3:
	case type_id_type::tid_command_message_amf0:
		OnCommandMessage( move( packet ), move( data ) );
		break;
	}
}

void NetConnection::OnNetworkMessage( const rtmp_packet packet, vector<uint8> data )
{
	switch( packet.type_id_ )
	{
	case type_id_type::tid_set_chunk_size:
		utility::convert_big_endian( data.data(), 4, &rxChunkSize_ );
		break;

	case type_id_type::tid_abort_message:
		break;

	case type_id_type::tid_acknowledgement:
		break;

	case type_id_type::tid_user_control_message:
		OnUserControlMessage( move( packet ), move( data ) );
		break;

	case type_id_type::tid_window_acknowledgement_size:
		utility::convert_big_endian( data.data(), 4, &rxWindowSize_ );
		break;

	case type_id_type::tid_set_peer_bandwidth:
		{
			uint32 buf;
			utility::convert_big_endian( data.data(), 4, &buf );
			//const auto limitType = static_cast<limit_type>( data[4] );
			WindowAcknowledgementSizeAsync( buf );
			break;
		}
	}
}

void NetConnection::OnUserControlMessage( const rtmp_packet /*packet*/, vector<uint8> data )
{
	uint16 buf;
	utility::convert_big_endian( data.data(), 2, &buf );

	const auto& messageType = static_cast<UserControlMessageEventType>( buf );
	switch( messageType )
	{
	case UserControlMessageEventType::StreamBegin:
		{
			uint32 streamId;
			utility::convert_big_endian( data.data() + 2, 4, &streamId );
			if( streamId == 0 )
				SetBufferLengthAsync( 0, DEFAULT_BUFFER_MILLSECONDS );
			break;
		}

	case UserControlMessageEventType::StreamEof:
	case UserControlMessageEventType::StreamDry:
	case UserControlMessageEventType::StreamIsRecorded:
		{
			//uint32 streamId;
			//utility::convert_big_endian( data.data() + 2, &streamId, 4 );
			break;
		}
	case UserControlMessageEventType::PingRequest:
		{
			uint32 timestamp;
			utility::convert_big_endian( data.data() + 2, 4, &timestamp );
			PingResponseAsync( timestamp );
			break;
		}
	}
}

void NetConnection::OnCommandMessage( const rtmp_packet /*packet*/, vector<uint8> data )
{
	const auto& amf = RtmpHelper::ParseAmf( move( data ) );
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
		const auto& itr = netStreamTemporary_.lower_bound( tid );
		if( itr != netStreamTemporary_.cend() && itr->first == tid )
		{
			if( name == "_result" )
			{
				//const auto commandBuf = amf->GetAt( 2 );
				//if( commandBuf->ValueType != Mntone::Data::Amf::AmfValueType::Object )
				//	const auto command = commandBuf->GetObject();
				const auto& sid = static_cast<uint32>( amf->GetNumberAt( 3 ) );

				auto stream = itr->second;
				stream->streamId_ = sid;
				bindingNetStream_.emplace( sid, stream );
				stream->AttachedImpl();
				SetBufferLengthAsync( sid, DEFAULT_BUFFER_MILLSECONDS );
			}
			netStreamTemporary_.erase( itr );
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

	vector<uint8> buf( 4 );
	utility::convert_big_endian( &chunkSize, 4, buf.data() );
	return SendNetworkAsync( type_id_type::tid_set_chunk_size, move( buf ) );
}

task<void> NetConnection::AbortMessageAsync( uint32 chunkStreamId )
{
	vector<uint8> buf( 4 );
	utility::convert_big_endian( &chunkStreamId, 4, buf.data() );
	return SendNetworkAsync( type_id_type::tid_abort_message, move( buf ) );
}

task<void> NetConnection::AcknowledgementAsync( uint32 sequenceNumber )
{
	vector<uint8> buf( 4 );
	utility::convert_big_endian( &sequenceNumber, 4, buf.data() );
	return SendNetworkAsync( type_id_type::tid_acknowledgement, move( buf ) );
}

task<void> NetConnection::WindowAcknowledgementSizeAsync( uint32 acknowledgementWindowSize )
{
	txWindowSize_ = acknowledgementWindowSize;

	vector<uint8> buf( 4 );
	utility::convert_big_endian( &acknowledgementWindowSize, 4, buf.data() );
	return SendNetworkAsync( type_id_type::tid_window_acknowledgement_size, move( buf ) );
}

task<void> NetConnection::SetPeerBandWidthAsync( uint32 windowSize, limit_type type )
{
	vector<uint8> buf( 5 );
	utility::convert_big_endian( &windowSize, 4, buf.data() );
	buf[4] = type;
	return SendNetworkAsync( type_id_type::tid_set_peer_bandwidth, move( buf ) );
}

task<void> NetConnection::SetBufferLengthAsync( const uint32 streamId, const uint32 bufferLength )
{
	vector<uint8> buf( 10 );
	utility::convert_big_endian( &streamId, 4, buf.data() + 2 );
	utility::convert_big_endian( &bufferLength, 4, buf.data() + 6 );
	return UserControlMessageEventAsync( UserControlMessageEventType::SetBufferLength, move( buf ) );
}

task<void> NetConnection::PingResponseAsync( const uint32 timestamp )
{
	vector<uint8> buf( 6 );
	utility::convert_big_endian( &timestamp, 4, buf.data() + 2 );
	return UserControlMessageEventAsync( UserControlMessageEventType::PingResponse, move( buf ) );
}

task<void> NetConnection::UserControlMessageEventAsync( UserControlMessageEventType type, vector<uint8> data )
{
	const auto& cType = static_cast<uint16>( type );
	utility::convert_big_endian( &cType, 2, data.data() );
	return SendNetworkAsync( type_id_type::tid_user_control_message, move( data ) );
}

task<void> NetConnection::SendNetworkAsync( const type_id_type type, const vector<uint8> data )
{
	rtmp_packet packet;
	packet.chunk_stream_id_ = 2; // for Network (2)
	packet.timestamp_ = utility::get_windows_time() - startTime_;
	packet.length_ = static_cast<uint32>( data.size() );
	packet.type_id_ = type;
	packet.stream_id_ = 0;

	return SendAsync( move( packet ), move( data ), true );
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
	packet.chunk_stream_id_ = 3; // for Action (3)
	packet.timestamp_ = utility::get_windows_time() - startTime_;
	packet.length_ = length;
	packet.type_id_ = type_id_type::tid_command_message_amf0;
	packet.stream_id_ = streamId;

	vector<uint8> buf( length );
	copy_n( amfData->begin() + 5, length, begin( buf ) );
	return SendAsync( move( packet ), move( buf ) );
}

task<void> NetConnection::SendAsync( rtmp_packet packet, const vector<uint8> data, const bool isFormatTypeZero )
{
	return create_task( [=] { SendImpl( move( packet ), move( data ), isFormatTypeZero ); } );
}

void NetConnection::SendImpl( rtmp_packet packet, const vector<uint8> data, const bool isFormatTypeZero )
{
	auto in_itr = data.cbegin();
	const auto chunk_size = txChunkSize_;
	const auto in_len = packet.length_;
	const auto in_end_before = data.cend() - in_len % chunk_size;
	const auto in_end = data.cend();

	// ---[ Header ]----------
	auto buf = CreateHeader( move( packet ), isFormatTypeZero );
	const auto header_size = buf.size();

	buf.resize( header_size + in_len + ( in_len - 1 ) / chunk_size );
	auto out_itr = buf.begin() + header_size;

	// ---[ Data ]----------
	bool odd = true;
	for( ; in_itr != in_end_before; in_itr += chunk_size, odd != odd )
	{
		copy_n( in_itr, chunk_size, out_itr );
		out_itr += chunk_size;
		*out_itr++ = odd ? 0xc3 : 0xc4;
	}
	copy_n( in_itr, in_len % chunk_size, out_itr );

	// ---[ Send ]----------
	connection_->Write( buf );
}

vector<uint8> NetConnection::CreateHeader( rtmp_packet packet, bool isFormatTypeZero )
{
	vector<uint8> data( 18 );
	auto ptr = data.data();

	// ---[ Get object ]----------
	shared_ptr<rtmp_packet> bakPacket;
	const auto& itr = txBakPackets_.lower_bound( packet.chunk_stream_id_ );
	if( itr != txBakPackets_.end() && itr->first == packet.chunk_stream_id_ )
		bakPacket = itr->second;
	else
		isFormatTypeZero = true;

	// ---[ Decide formatType ]----------
	uint8 formatType;
	if( isFormatTypeZero )
		formatType = 0;
	else if( packet.stream_id_ == bakPacket->stream_id_ )
	{
		if( packet.type_id_ == bakPacket->type_id_ && packet.length_ == bakPacket->length_ )
		{
			if( packet.timestamp_ == bakPacket->timestamp_ + 2 * bakPacket->timestamp_delta_ )
			{
				formatType = 3;
				packet.timestamp_delta_ = bakPacket->timestamp_delta_;
			}
			else if( packet.timestamp_ < bakPacket->timestamp_ )
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
	if( packet.chunk_stream_id_ < 64 )
	{
		*ptr++ |= packet.chunk_stream_id_; //& 0x3f;
	}
	else if( packet.chunk_stream_id_ < 320 )
	{
		ptr[1] = static_cast<uint8>( packet.chunk_stream_id_ - 64 );
		ptr += 2;
	}
	else if( packet.chunk_stream_id_ < 65600 )
	{
		*ptr++ |= 1;
		uint16 buf = packet.chunk_stream_id_ - 64;
		utility::convert_big_endian( &buf, 2, ptr );
		ptr += 2;
	}
	else
		throw ref new Platform::InvalidArgumentException();

	// ---[ Chunk message header ]----------
	switch( formatType )
	{
	case 0:
		utility::convert_big_endian( &packet.length_, 3, ptr + 3 );
		ptr[6] = static_cast<uint8>( packet.type_id_ );
		utility::convert_little_endian( &packet.stream_id_, 4, ptr + 7 ); // LE
		if( packet.timestamp_ >= 0xffffff )
		{
			ptr[0] = ptr[1] = ptr[2] = 0xff;
			ptr += 11;

			utility::convert_big_endian( &packet.timestamp_, 4, ptr );
			ptr += 4;
		}
		else
		{
			utility::convert_big_endian( &packet.timestamp_, 3, ptr );
			ptr += 11;
		}
		break;
	case 1:
		{
			packet.timestamp_delta_ = packet.timestamp_ - bakPacket->timestamp_;
			utility::convert_big_endian( &packet.length_, 3, ptr + 3 );
			ptr[6] = static_cast<uint8>( packet.type_id_ );
			if( packet.timestamp_delta_ >= 0xffffff )
			{
				ptr[0] = ptr[1] = ptr[2] = 0xff;
				ptr += 7;

				utility::convert_big_endian( &packet.timestamp_delta_, 4, ptr );
				ptr += 4;
			}
			else
			{
				utility::convert_big_endian( &packet.timestamp_delta_, 3, ptr );
				ptr += 7;
			}
			break;
		}
	case 2:
		{
			packet.timestamp_delta_ = packet.timestamp_ - bakPacket->timestamp_;
			if( packet.timestamp_delta_ >= 0xffffff )
			{
				ptr[0] = ptr[1] = ptr[2] = 0xff;
				ptr += 3;

				utility::convert_big_endian( &packet.timestamp_delta_, 4, ptr );
				ptr += 4;
			}
			else
			{
				utility::convert_big_endian( &packet.timestamp_delta_, 3, ptr );
				ptr += 3;
			}
			break;
		}
	case 3:		
		if( packet.timestamp_delta_ >= 0xffffff )
		{
			utility::convert_big_endian( &packet.timestamp_delta_, 4, ptr );
			ptr += 4;
		}
		break;
	}

	data.resize( ptr - data.data() );
	txBakPackets_.emplace( packet.chunk_stream_id_, make_shared<rtmp_packet>( move( packet ) ) );
	return move( data );
}

#pragma endregion