#include "pch.h"
#include "Connection.h"

using namespace Mntone::Rtmp;

Connection::Connection( void ) :
	_IsInitialized( false )
{ }

Connection::~Connection( void )
{
	delete _dataReader;
	delete _dataWriter;
	delete _streamSocket;
}

void Connection::Connect( Platform::String^ host, Platform::String^ port )
{
	using namespace Windows::Networking;
	using namespace Windows::Networking::Sockets;
	using namespace Windows::Storage::Streams;

	_streamSocket = ref new StreamSocket();
	auto ex = _streamSocket->ConnectAsync( ref new HostName( host ), port, SocketProtectionLevel::PlainSocket );
	create_task( ex ).wait();

	_dataReader = ref new DataReader( _streamSocket->InputStream );
	_dataReader->InputStreamOptions = InputStreamOptions::Partial;
	_dataWriter = ref new DataWriter( _streamSocket->OutputStream );

	_IsInitialized = true;
}

uint32 Connection::TryRead( uint8 *const data, const size_t length )
{
	auto ex = _dataReader->LoadAsync( static_cast<uint32>( length ) );
	auto actualLength = create_task( ex ).get();
	if( actualLength != 0 )
		_dataReader->ReadBytes( Platform::ArrayReference<uint8>( data, actualLength ) );
	return actualLength;
}

void Connection::Read( uint8 *const data, const size_t length )
{
	auto ex = _dataReader->LoadAsync( static_cast<uint32>( length ) );
	auto actualLength = create_task( ex ).get();
	if( actualLength != 0 )
		_dataReader->ReadBytes( Platform::ArrayReference<uint8>( data, actualLength ) );

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

void Connection::Write( uint8 *const data, const size_t length )
{
	_dataWriter->WriteBytes( Platform::ArrayReference<uint8>( data, static_cast<uint32>( length ) ) );
	auto ex = _dataWriter->StoreAsync();
	task<uint32_t>( ex ).wait();
}

void Connection::Write( std::vector<uint8>& data )
{
	Write( data.data(), data.size() );
}

void Connection::Write( std::vector<uint8>& data, const size_t length )
{
	Write( data.data(), length );
}