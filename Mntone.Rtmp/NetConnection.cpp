#include "pch.h"
#include "NetConnection.h"
#include "NetStream.h"
#include "RtmpHelper.h"
#include "Command/NetConnectionConnectCommand.h"

using namespace std;
using namespace Concurrency;
using namespace mntone::rtmp;
using namespace Mntone::Rtmp;
namespace WF = Windows::Foundation;

const auto DEFAULT_WINDOW_SIZE = 2500000;
const auto DEFAULT_CHUNK_SIZE = 128;
const auto DEFAULT_BUFFER_MILLSECONDS = 5000;

NetConnection::NetConnection()
	: connection_( ref new Connection() )
	, latestTransactionId_( 2 )
	, receiveOperation_( nullptr )
	, rxHeaderBuffer_( 11 )
	, rxWindowSize_( DEFAULT_WINDOW_SIZE ), txWindowSize_( DEFAULT_WINDOW_SIZE )
	, rxChunkSize_( DEFAULT_CHUNK_SIZE ), txChunkSize_( DEFAULT_CHUNK_SIZE )
{
	connection_->ReadOperationChanged += ref new WF::TypedEventHandler<Connection^, WF::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^>( this, &NetConnection::OnReadOperationChanged );
}

void NetConnection::CloseImpl()
{
	if( receiveOperation_ != nullptr )
	{
		receiveOperation_->Cancel();
		receiveOperation_ = nullptr;
	}
	if( connection_ != nullptr )
	{
		delete connection_;
		connection_ = nullptr;
	}
	Closed( this, ref new NetConnectionClosedEventArgs() );
}
	
WF::IAsyncAction^ NetConnection::ConnectAsync( WF::Uri^ uri )
{
	return ConnectAsync( ref new RtmpUri( uri ) );
}

WF::IAsyncAction^ NetConnection::ConnectAsync( WF::Uri^ uri, Command::NetConnectionConnectCommand^ command )
{
	if( command->Type != "connect" )
		throw ref new Platform::InvalidArgumentException();

	return ConnectAsync( ref new RtmpUri( uri ), command );
}

WF::IAsyncAction^ NetConnection::ConnectAsync( RtmpUri^ uri )
{
	auto connect = ref new Command::NetConnectionConnectCommand( uri->App );
	connect->TcUrl = uri->ToString();
	return ConnectAsync( uri, connect );
}

WF::IAsyncAction^ NetConnection::ConnectAsync( RtmpUri^ uri, Command::NetConnectionConnectCommand^ command )
{
	startTime_ = utility::get_windows_time();
	Uri_ = uri;

	return create_async( [this, command]
	{
		auto task = connection_->ConnectAsync( Uri_->Host, Uri_->Port.ToString() );
		return task.then( [this, command]
		{
			Handshake( ref new HandshakeCallbackHandler( [this, command]
			{
				SendActionAsync( command->Commandify() );
				Receive();
			} ) );
		}, task_continuation_context::use_arbitrary() );
	} );
}

WF::IAsyncAction^ NetConnection::CallAsync( Command::NetConnectionCallCommand^ command )
{
	return create_async( [this, command]
	{
		const auto tid = latestTransactionId_++;
		command->TransactionId = tid;
		return SendActionAsync( 0, command->Commandify() );
	} );
}

task<void> NetConnection::AttachNetStreamAsync( NetStream^ stream )
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

void NetConnection::OnReadOperationChanged( Connection^ sender, WF::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^ operation )
{
	receiveOperation_ = operation;
}

void NetConnection::Receive()
{
	connection_->Read( 1, ref new ConnectionCallbackHandler( this, &NetConnection::ReceiveHeader1Impl ) );
}

void NetConnection::ReceiveHeader1Impl( const std::vector<uint8> result )
{
	const uint8_t& format_type = ( result[0] >> 6 ) & 0x03;
	const uint16& chunk_stream_id = result[0] & 0x3f;
	if( chunk_stream_id == 0 )
	{
		connection_->Read( 1, ref new ConnectionCallbackHandler( [this, format_type]( const std::vector<uint8> result )
		{
			uint16 chunk_stream_id = result[0] + 64;
			ReceiveHeader2Impl( format_type, chunk_stream_id );
		} ) );
	}
	else if( chunk_stream_id == 1 )
	{
		connection_->Read( 2, ref new ConnectionCallbackHandler( [this, format_type]( const std::vector<uint8> result )
		{
			uint16 chunk_stream_id;
			utility::convert_big_endian( &result[0], 2, &chunk_stream_id );
			chunk_stream_id += 64;
			ReceiveHeader2Impl( format_type, chunk_stream_id );
		} ) );
	}
	else
	{
		ReceiveHeader2Impl( format_type, chunk_stream_id );
	}
}

void NetConnection::ReceiveHeader2Impl( const uint8 format_type, const uint16 chunk_stream_id )
{
	// ---[ Get object ]----------
	shared_ptr<rtmp_packet> packet;
	const auto& itr = rxBakPackets_.lower_bound( chunk_stream_id );
	if( itr != rxBakPackets_.end() && itr->first == chunk_stream_id )
	{
		packet = itr->second;
	}
	else
	{
		packet = make_shared<rtmp_packet>();
		packet->chunk_stream_id_ = chunk_stream_id;
		packet->length_ = 0;
		rxBakPackets_.emplace_hint( itr, chunk_stream_id, packet );
	}

	// ---[ Read header body ]----------
	switch( format_type )
	{
	case 0:
		connection_->Read( 11, ref new ConnectionCallbackHandler( [this, packet]( const std::vector<uint8> result )
		{
			( &packet->timestamp_ )[3] = 0; // initialize
			utility::convert_big_endian( &result[0], 3, &packet->timestamp_ );
			utility::convert_big_endian( &result[3], 3, &packet->length_ );
			packet->type_id_ = static_cast<type_id_type>( result[6] );
			utility::convert_little_endian( &result[7], 4, &packet->stream_id_ ); // LE

			if( packet->timestamp_ == 0xffffff )
			{
				connection_->Read( 4, ref new ConnectionCallbackHandler( [this, packet]( const std::vector<uint8> result )
				{
					utility::convert_big_endian( &result[0], 4, &packet->timestamp_ );
					packet->timestamp_delta_ = packet->timestamp_;
					ReceiveBodyImpl( packet );
				} ) );
			}
			else
			{
				packet->timestamp_delta_ = packet->timestamp_;
				ReceiveBodyImpl( packet );
			}
		} ) );
		break;
	
	case 1:
		connection_->Read( 7, ref new ConnectionCallbackHandler( [this, packet]( const std::vector<uint8> result )
		{
			( &packet->timestamp_delta_ )[3] = 0; // initialize
			utility::convert_big_endian( &result[0], 3, &packet->timestamp_delta_ );
			utility::convert_big_endian( &result[3], 3, &packet->length_ );
			packet->type_id_ = static_cast<type_id_type>( result[6] );

			if( packet->timestamp_delta_ == 0xffffff )
			{
				connection_->Read( 4, ref new ConnectionCallbackHandler( [this, packet]( const std::vector<uint8> result )
				{
					utility::convert_big_endian( &result[0], 4, &packet->timestamp_delta_ );
					packet->timestamp_ += packet->timestamp_delta_;
					ReceiveBodyImpl( packet );
				} ) );
			}
			else
			{
				packet->timestamp_ += packet->timestamp_delta_;
				ReceiveBodyImpl( packet );
			}
		} ) );
		break;
	
	case 2:
		connection_->Read( 3, ref new ConnectionCallbackHandler( [this, packet]( const std::vector<uint8> result )
		{
			( &packet->timestamp_delta_ )[3] = 0; // initialize
			utility::convert_big_endian( &result[0], 3, &packet->timestamp_delta_ );

			if( packet->timestamp_delta_ == 0xffffff )
			{
				connection_->Read( 4, ref new ConnectionCallbackHandler( [this, packet]( const std::vector<uint8> result )
				{
					utility::convert_big_endian( &result[0], 4, &packet->timestamp_delta_ );
					packet->timestamp_ += packet->timestamp_delta_;
					ReceiveBodyImpl( packet );
				} ) );
			}
			else
			{
				packet->timestamp_ += packet->timestamp_delta_;
				ReceiveBodyImpl( packet );
			}
		} ) );
		break;
	
	case 3:
		if( packet->timestamp_delta_ > 0xffffff )
		{
			connection_->Read( 4, ref new ConnectionCallbackHandler( [this, packet]( const std::vector<uint8> result )
			{
				utility::convert_big_endian( &result[0], 4, &packet->timestamp_delta_ );
				packet->timestamp_ += packet->timestamp_delta_;
				ReceiveBodyImpl( packet );
			} ) );
		}
		else
		{
			packet->timestamp_ += packet->timestamp_delta_;
			ReceiveBodyImpl( packet );
		}
		break;
	}
}

void NetConnection::ReceiveBodyImpl( const shared_ptr<rtmp_packet> packet )
{
	const auto& data_length = packet->length_ + ( packet->length_ - 1 ) / rxChunkSize_;
	connection_->Read( data_length, ref new ConnectionCallbackHandler( [this, packet]( std::vector<uint8> result )
	{
		const auto& length = result.size();

		if( length > rxChunkSize_ )
		{
			const auto chunk_size_plus_one = rxChunkSize_ + 1;
			auto from_itr = result.cbegin() + chunk_size_plus_one;
			const auto from_end_before = result.cend() - length % chunk_size_plus_one;
			auto to_itr = result.begin() + rxChunkSize_;
			for( ; from_itr != from_end_before; from_itr += chunk_size_plus_one, to_itr += rxChunkSize_ )
			{
				copy_n( from_itr, rxChunkSize_, to_itr );
			}
			copy_n( from_itr, length % chunk_size_plus_one, to_itr );
			result.resize( packet->length_ );
		}
		ReceiveCallbackImpl( packet, std::move( result ) );
	} ) );
}

void NetConnection::ReceiveCallbackImpl( const shared_ptr<rtmp_packet> packet, const std::vector<uint8> result )
{
	const auto& sid = packet->stream_id_;
	if( sid == 0 )
	{
		OnMessage( *packet.get(), move( result ) );
	}
	else
	{
		const auto& itr = bindingNetStream_.lower_bound( sid );
		if( itr != bindingNetStream_.cend() && itr->first == sid )
			itr->second->OnMessage( *packet.get(), move( result ) );
	}
	Receive();
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
		utility::convert_big_endian( &data[0], 4, &rxChunkSize_ );
		break;

	case type_id_type::tid_abort_message:
		break;

	case type_id_type::tid_acknowledgement:
		break;

	case type_id_type::tid_user_control_message:
		OnUserControlMessage( move( packet ), move( data ) );
		break;

	case type_id_type::tid_window_acknowledgement_size:
		utility::convert_big_endian( &data[0], 4, &rxWindowSize_ );
		break;

	case type_id_type::tid_set_peer_bandwidth:
		{
			uint32 buf;
			utility::convert_big_endian( &data[0], 4, &buf );
			//const auto limitType = static_cast<limit_type>( data[4] );
			WindowAcknowledgementSizeAsync( buf );
			break;
		}
	}
}

void NetConnection::OnUserControlMessage( const rtmp_packet /*packet*/, vector<uint8> data )
{
	uint16 buf;
	utility::convert_big_endian( &data[0], 2, &buf );

	const auto& message_type = static_cast<UserControlMessageEventType>( buf );
	switch( message_type )
	{
	case UserControlMessageEventType::StreamBegin:
		{
			uint32 stream_id;
			utility::convert_big_endian( &data[2], 4, &stream_id );
			if( stream_id == 0 )
				SetBufferLengthAsync( 0, DEFAULT_BUFFER_MILLSECONDS );
			break;
		}

	case UserControlMessageEventType::StreamEof:
		{
			//uint32 stream_id;
			//utility::convert_big_endian( &data[2], 4, &stream_id );
			break;
		}

	case UserControlMessageEventType::StreamDry:
	case UserControlMessageEventType::StreamIsRecorded:
		{
			//uint32 stream_id;
			//utility::convert_big_endian( &data[2], 4, &stream_id );
			break;
		}

	case UserControlMessageEventType::PingRequest:
		{
			uint32 timestamp;
			utility::convert_big_endian( &data[2], 4, &timestamp );
			PingResponseAsync( timestamp );
			break;
		}
	}
}

void NetConnection::OnCommandMessage( const rtmp_packet /*packet*/, vector<uint8> data )
{
	const auto& amf = RtmpHelper::ParseAmf( move( data ) );
	if( amf == nullptr )
		return;

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
		const auto response = amf->GetAt( 3 );
		Callback( this, ref new NetConnectionCallbackEventArgs( name, response ) );
	}
}

#pragma endregion

#pragma region Network operation (Client to Server)

task<void> NetConnection::SetChunkSizeAsync( const uint32 chunkSize )
{
	if( chunkSize > 0x7fffffff )
		throw ref new Platform::InvalidArgumentException( "Invalid chunkSize. Valid chunkSize is 1 to 2147483647." );

	txChunkSize_ = chunkSize;

	vector<uint8> buf( 4 );
	utility::convert_big_endian( &chunkSize, 4, &buf[0] );
	return SendNetworkAsync( type_id_type::tid_set_chunk_size, move( buf ) );
}

task<void> NetConnection::AbortMessageAsync( const uint32 chunkStreamId )
{
	vector<uint8> buf( 4 );
	utility::convert_big_endian( &chunkStreamId, 4, &buf[0] );
	return SendNetworkAsync( type_id_type::tid_abort_message, move( buf ) );
}

task<void> NetConnection::AcknowledgementAsync( const uint32 sequenceNumber )
{
	vector<uint8> buf( 4 );
	utility::convert_big_endian( &sequenceNumber, 4, &buf[0] );
	return SendNetworkAsync( type_id_type::tid_acknowledgement, move( buf ) );
}

task<void> NetConnection::WindowAcknowledgementSizeAsync( const uint32 acknowledgementWindowSize )
{
	txWindowSize_ = acknowledgementWindowSize;

	vector<uint8> buf( 4 );
	utility::convert_big_endian( &acknowledgementWindowSize, 4, &buf[0] );
	return SendNetworkAsync( type_id_type::tid_window_acknowledgement_size, move( buf ) );
}

task<void> NetConnection::SetPeerBandWidthAsync( const uint32 windowSize, const limit_type type )
{
	vector<uint8> buf( 5 );
	utility::convert_big_endian( &windowSize, 4, &buf[0] );
	buf[4] = type;
	return SendNetworkAsync( type_id_type::tid_set_peer_bandwidth, move( buf ) );
}

task<void> NetConnection::SetBufferLengthAsync( const uint32 streamId, const uint32 bufferLength )
{
	vector<uint8> buf( 10 );
	utility::convert_big_endian( &streamId, 4, &buf[2] );
	utility::convert_big_endian( &bufferLength, 4, &buf[6] );
	return UserControlMessageEventAsync( UserControlMessageEventType::SetBufferLength, move( buf ) );
}

task<void> NetConnection::PingResponseAsync( const uint32 timestamp )
{
	vector<uint8> buf( 6 );
	utility::convert_big_endian( &timestamp, 4, &buf[2] );
	return UserControlMessageEventAsync( UserControlMessageEventType::PingResponse, move( buf ) );
}

task<void> NetConnection::UserControlMessageEventAsync( UserControlMessageEventType type, vector<uint8> data )
{
	const auto& c_type = static_cast<uint16>( type );
	utility::convert_big_endian( &c_type, 2, &data[0] );
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

task<void> NetConnection::SendActionAsync( const uint32 streamId, Mntone::Data::Amf::AmfArray^ amf )
{
	auto amf_data = amf->Sequencify( Mntone::Data::Amf::AmfEncodingType::Amf0 );
	const auto& length = amf_data->Length - 5;

	rtmp_packet packet;
	packet.chunk_stream_id_ = 3; // for Action (3)
	packet.timestamp_ = utility::get_windows_time() - startTime_;
	packet.length_ = length;
	packet.type_id_ = type_id_type::tid_command_message_amf0;
	packet.stream_id_ = streamId;

	vector<uint8> buf( length );
	copy_n( amf_data->begin() + 5, length, buf.begin() );
	return SendAsync( move( packet ), move( buf ) );
}

task<void> NetConnection::SendAsync( rtmp_packet packet, const vector<uint8> data, const bool isFormatTypeZero )
{
	auto in_itr = data.cbegin();
	const auto chunk_size = txChunkSize_;
	const auto in_len = packet.length_;
	const auto in_end_before = data.cend() - in_len % chunk_size;

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
	return connection_->Write( buf );
}

vector<uint8> NetConnection::CreateHeader( rtmp_packet packet, bool isFormatTypeZero )
{
	vector<uint8> data( 18 );
	auto itr = data.begin();

	// ---[ Get object ]----------
	shared_ptr<rtmp_packet> bak_packet;
	const auto& pk_itr = txBakPackets_.lower_bound( packet.chunk_stream_id_ );
	if( pk_itr != txBakPackets_.end() && pk_itr->first == packet.chunk_stream_id_ )
		bak_packet = pk_itr->second;
	else
		isFormatTypeZero = true;

	// ---[ Decide formatType ]----------
	uint8 formatType;
	if( isFormatTypeZero )
		formatType = 0;
	else if( packet.stream_id_ == bak_packet->stream_id_ )
	{
		if( packet.type_id_ == bak_packet->type_id_ && packet.length_ == bak_packet->length_ )
		{
			if( packet.timestamp_ == bak_packet->timestamp_ + 2 * bak_packet->timestamp_delta_ )
			{
				formatType = 3;
				packet.timestamp_delta_ = bak_packet->timestamp_delta_;
			}
			else if( packet.timestamp_ < bak_packet->timestamp_ )
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
	*itr = formatType << 6;
	if( packet.chunk_stream_id_ < 64 )
	{
		*itr++ |= packet.chunk_stream_id_; //& 0x3f;
	}
	else if( packet.chunk_stream_id_ < 320 )
	{
		itr[1] = static_cast<uint8>( packet.chunk_stream_id_ - 64 );
		itr += 2;
	}
	else if( packet.chunk_stream_id_ < 65600 )
	{
		*itr++ |= 1;

		uint16 buf = packet.chunk_stream_id_ - 64;
		utility::convert_big_endian( &buf, 2, &itr[0] );
		itr += 2;
	}
	else
		throw ref new Platform::InvalidArgumentException();

	// ---[ Chunk message header ]----------
	switch( formatType )
	{
	case 0:
		utility::convert_big_endian( &packet.length_, 3, &itr[3] );
		itr[6] = static_cast<uint8>( packet.type_id_ );
		utility::convert_little_endian( &packet.stream_id_, 4, &itr[7] ); // LE
		if( packet.timestamp_ >= 0xffffff )
		{
			itr[0] = itr[1] = itr[2] = 0xff;
			itr += 11;

			utility::convert_big_endian( &packet.timestamp_, 4, &itr[0] );
			itr += 4;
		}
		else
		{
			utility::convert_big_endian( &packet.timestamp_, 3, &itr[0] );
			itr += 11;
		}
		break;
	case 1:
		{
			packet.timestamp_delta_ = packet.timestamp_ - bak_packet->timestamp_;
			utility::convert_big_endian( &packet.length_, 3, &itr[3] );
			itr[6] = static_cast<uint8>( packet.type_id_ );
			if( packet.timestamp_delta_ >= 0xffffff )
			{
				itr[0] = itr[1] = itr[2] = 0xff;
				itr += 7;

				utility::convert_big_endian( &packet.timestamp_delta_, 4, &itr[0] );
				itr += 4;
			}
			else
			{
				utility::convert_big_endian( &packet.timestamp_delta_, 3, &itr[0] );
				itr += 7;
			}
			break;
		}
	case 2:
		{
			packet.timestamp_delta_ = packet.timestamp_ - bak_packet->timestamp_;
			if( packet.timestamp_delta_ >= 0xffffff )
			{
				itr[0] = itr[1] = itr[2] = 0xff;
				itr += 3;

				utility::convert_big_endian( &packet.timestamp_delta_, 4, &itr[0] );
				itr += 4;
			}
			else
			{
				utility::convert_big_endian( &packet.timestamp_delta_, 3, &itr[0] );
				itr += 3;
			}
			break;
		}
	case 3:		
		if( packet.timestamp_delta_ >= 0xffffff )
		{
			utility::convert_big_endian( &packet.timestamp_delta_, 4, &itr[0] );
			itr += 4;
		}
		break;
	}

	data.resize( itr - data.cbegin() );
	txBakPackets_.emplace( packet.chunk_stream_id_, make_shared<rtmp_packet>( move( packet ) ) );
	return move( data );
}

#pragma endregion