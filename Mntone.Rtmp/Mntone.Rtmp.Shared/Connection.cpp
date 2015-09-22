#include "pch.h"
#include "Connection.h"

using namespace Concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Mntone::Rtmp;

Connection::Connection()
	: IsInitialized_( false )
	, streamSocket_( nullptr )
	, dataWriter_( nullptr )
{ }

Connection::~Connection()
{
	if( dataWriter_ != nullptr )
	{
		dataWriter_->DetachStream();
		delete dataWriter_;
		dataWriter_ = nullptr;
	}
}

task<void> Connection::ConnectAsync( RtmpUri^ uri )
{
	streamSocket_ = ref new StreamSocket();
	streamSocket_->Control->KeepAlive = true;
	streamSocket_->Control->QualityOfService = SocketQualityOfService::LowLatency;
	auto task = streamSocket_->ConnectAsync(
		ref new HostName( uri->Host ),
		uri->Port.ToString(),
		uri->Scheme == RtmpScheme::Rtmps ? SocketProtectionLevel::Tls12 : SocketProtectionLevel::PlainSocket );
	return create_task( task ).then( [this]
	{
		dataWriter_ = ref new Windows::Storage::Streams::DataWriter( streamSocket_->OutputStream );

		IsInitialized_ = true;
	} );
}

void Connection::Read( const uint32 length, ConnectionCallbackHandler^ callbackFunction )
{
	using namespace Windows::Storage::Streams;

	auto buffer = ref new Buffer( length );
	auto read_operation = streamSocket_->InputStream->ReadAsync( buffer, length, InputStreamOptions::Partial );
	read_operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<IBuffer^, uint32>(
	[this, length, callbackFunction]( IAsyncOperationWithProgress<IBuffer^, uint32>^ operation, AsyncStatus status )
	{
		if( status == AsyncStatus::Completed )
		{
			auto buffer = operation->GetResults();
			if( buffer->Length != length )
			{
				ContinuousRead( buffer, length - buffer->Length, callbackFunction );
			}
			else
			{
				callbackFunction( buffer );
			}
		}
	} );
	ReadOperationChanged( this, read_operation );
}

void Connection::ContinuousRead( Windows::Storage::Streams::IBuffer^ data, const uint32 length, ConnectionCallbackHandler^ callbackFunction )
{
	using namespace Windows::Storage::Streams;

	auto buffer = ref new Buffer( length );
	auto read_operation = streamSocket_->InputStream->ReadAsync( buffer, length, InputStreamOptions::Partial );
	read_operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<IBuffer^, uint32>(
		[this, data, length, callbackFunction]( IAsyncOperationWithProgress<IBuffer^, uint32>^ operation, AsyncStatus status )
	{
		if( status == AsyncStatus::Completed )
		{
			auto writer = ref new DataWriter();
			writer->WriteBuffer( data );

			auto buffer = operation->GetResults();
			writer->WriteBuffer( buffer );
			if( buffer->Length != length )
			{
				ContinuousRead( writer->DetachBuffer(), length - buffer->Length, callbackFunction );
			}
			else
			{
				callbackFunction( writer->DetachBuffer() );
			}
		}
	} );
	ReadOperationChanged( this, read_operation );
}

task<void> Connection::Write( const uint8* const data, const size_t length )
{
	dataWriter_->WriteBytes( Platform::ArrayReference<uint8>( const_cast<uint8_t*>( data ), static_cast<uint32>( length ) ) );
	return create_task( dataWriter_->StoreAsync() ).then( [] ( const uint32 ) { } );
}

void Connection::CloseImpl()
{
	if( dataWriter_ != nullptr )
	{
		delete dataWriter_;
		dataWriter_ = nullptr;
	}
	if( streamSocket_ != nullptr )
	{
		delete streamSocket_;
		streamSocket_ = nullptr;
	}
}