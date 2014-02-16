#include "pch.h"
#include "Connection.h"

using namespace Concurrency;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Mntone::Rtmp;

Connection::Connection()
	: IsInitialized_( false )
{ }

task<void> Connection::ConnectAsync( Platform::String^ host, Platform::String^ port )
{
	using namespace Windows::Networking;
	using namespace Windows::Networking::Sockets;

	streamSocket_ = ref new StreamSocket();
	auto task = streamSocket_->ConnectAsync( ref new HostName( host ), port, SocketProtectionLevel::PlainSocket );
	return create_task( task ).then( [=]
	{
		dataReader_ = ref new DataReader( streamSocket_->InputStream );
		dataReader_->InputStreamOptions = InputStreamOptions::Partial;
		dataWriter_ = ref new DataWriter( streamSocket_->OutputStream );

		IsInitialized_ = true;
	} );
}

void Connection::Read( const uint32 length, ConnectionCallbackHandler^ callbackFunction )
{
	auto buffer = ref new Buffer( length );
	auto read_operation = streamSocket_->InputStream->ReadAsync( buffer, length, InputStreamOptions::Partial );
	read_operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<IBuffer^, uint32>(
	[this, length, callbackFunction]( IAsyncOperationWithProgress<IBuffer^, uint32>^ operation, AsyncStatus status )
	{
		if( status == AsyncStatus::Completed )
		{
			auto buffer = operation->GetResults();
			auto reader = DataReader::FromBuffer( buffer );
			const auto& actual_length = reader->UnconsumedBufferLength;
			if( actual_length != 0 )
			{
				std::vector<uint8> data( actual_length );
				reader->ReadBytes( Platform::ArrayReference<uint8>( &data[0], actual_length ) );
				if( length == actual_length )
				{
					callbackFunction( std::move( data ) );
				}
				else
				{
					ContinuousRead( std::move( data ), actual_length, length - actual_length, callbackFunction );
				}
			}
		}
	} );
	ReadOperationChanged( this, read_operation );
}

void Connection::ContinuousRead( std::vector<uint8> data, const uint32 offset, const uint32 length, ConnectionCallbackHandler^ callbackFunction )
{
	auto buffer = ref new Buffer( length );
	auto read_operation = streamSocket_->InputStream->ReadAsync( buffer, length, InputStreamOptions::Partial );
	read_operation->Completed = ref new AsyncOperationWithProgressCompletedHandler<IBuffer^, uint32>(
	[=]( IAsyncOperationWithProgress<IBuffer^, uint32>^ operation, AsyncStatus status )
	{
		if( status == AsyncStatus::Completed )
		{
			auto buffer = operation->GetResults();
			auto reader = DataReader::FromBuffer( buffer );
			const auto& actual_length = reader->UnconsumedBufferLength;
			if( actual_length != 0 )
			{
				std::vector<uint8> new_data( offset + length );
				std::copy_n( data.cbegin(), offset, new_data.begin() );
				reader->ReadBytes( Platform::ArrayReference<uint8>( &new_data[offset], actual_length ) );
				if( length == actual_length )
				{
					callbackFunction( std::move( new_data ) );
				}
				else
				{
					ContinuousRead( std::move( new_data ), offset + actual_length, length - actual_length, callbackFunction );
				}
			}
		}
	} );
	ReadOperationChanged( this, read_operation );
}

Concurrency::task<void> Connection::Write( const uint8* const data, const size_t length )
{
	dataWriter_->WriteBytes( Platform::ArrayReference<uint8>( const_cast<uint8_t*>( data ), static_cast<uint32>( length ) ) );
	return Concurrency::create_task( dataWriter_->StoreAsync() ).then( [] ( const uint32 ) { } );
}

Concurrency::task<void> Connection::Write( const std::vector<uint8>& data )
{
	return Write( data.data(), data.size() );
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