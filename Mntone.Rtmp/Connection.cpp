#include "pch.h"
#include "Connection.h"

using namespace Mntone::Rtmp;

Connection::Connection()
	: IsInitialized_( false )
{ }

Connection::~Connection()
{
	delete dataReader_;
	delete dataWriter_;
	delete streamSocket_;
}

task<void> Connection::ConnectAsync( Platform::String^ host, Platform::String^ port )
{
	using namespace Windows::Networking;
	using namespace Windows::Networking::Sockets;
	using namespace Windows::Storage::Streams;

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

uint32 Connection::TryRead( uint8* const data, const size_t length )
{
	auto ex = dataReader_->LoadAsync( static_cast<uint32>( length ) );
	auto actualLength = create_task( ex ).get();
	if( actualLength != 0 )
		dataReader_->ReadBytes( Platform::ArrayReference<uint8>( data, actualLength ) );
	return actualLength;
}

void Connection::Read( uint8* const data, const size_t length )
{
	auto ex = dataReader_->LoadAsync( static_cast<uint32>( length ) );
	auto actualLength = create_task( ex ).get();
	if( actualLength != 0 )
		dataReader_->ReadBytes( Platform::ArrayReference<uint8>( data, actualLength ) );

	if( actualLength < length )
		Read( data + actualLength, length - actualLength );
}

void Connection::Read( std::vector<uint8>& data )
{
	Read( data.data(), data.size() );
}

void Connection::Read( std::vector<uint8>& data, const size_t length )
{
	Read( data.data(), length );
}

void Connection::Write( const uint8* const data, const size_t length )
{
	dataWriter_->WriteBytes( Platform::ArrayReference<uint8>( const_cast<uint8_t*>( data ), static_cast<uint32>( length ) ) );
	auto ex = dataWriter_->StoreAsync();
	create_task( ex ).get();
}

void Connection::Write( const std::vector<uint8>& data )
{
	Write( data.data(), data.size() );
}

void Connection::Write( const std::vector<uint8>& data, const size_t length )
{
	Write( data.data(), length );
}