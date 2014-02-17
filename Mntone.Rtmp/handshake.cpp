#include "pch.h"
#include "NetConnection.h"
#if !_DEBUG
#include <random>
#endif

using namespace mntone::rtmp;
using namespace Mntone::Rtmp;

const auto hs0_size = 1u;
const auto hsr_size = 1528u;
const auto hs1_size = 8u + hsr_size;
const auto hs2_size = 8u + hsr_size;

void NetConnection::Handshake( HandshakeCallbackHandler^ callbackFunction )
{
	// ---[ Send C0+C1 packet ]----------
	std::vector<uint8> send_data( hs0_size + hs1_size, 0xff );

	// C0 --- protcolVersion: uint8
	send_data[0] = 0x03; // default: 0x03: plain (or 0x06, 0x08, 0x09: encrypted)

	// C1 --- time: uint32, zero: uint32, random_data: 1528 bytes
	// [time]
	auto time = utility::hundred_nano_to_milli( utility::get_windows_time() - startTime_ ) + 1;
	utility::convert_big_endian( &time, 4, &send_data[1] );

	// [zero]
	std::fill_n( send_data.begin() + 5, 4, 0x00 );

	// [randomData_]
#if NDEBUG
	std::mt19937 engine;
	std::uniform_int_distribution<uint64_t> distribution( 0x0000000000000000, 0xffffffffffffffff );
	auto sptr = reinterpret_cast<uint64_t*>( &send_data[9] );
	const auto eptr = sptr + hsr_size / 8u;
	for( auto ptr = sptr; ptr != eptr; ++ptr )
		*ptr = distribution( engine );
#endif

	connection_->Write( send_data );

	// ---[ Receive S0+S1 packet ]----------
	connection_->Read( hs0_size + hs1_size, ref new ConnectionCallbackHandler( [=]( std::vector<uint8> result )
	{
		// S0 --- protocolVersion: uint8
		if( result[0] != 0x03 )
			throw ref new Platform::FailureException();

		// S1 --- time: uint32, zero: uint32, random_data: 1528 bytes

		// ---[ Send C2 packet ]----------
		// Create C2 data
		std::vector<uint8> cs2( hs2_size );
		memcpy( &cs2[0], &result[1], 4 );					// time
		utility::convert_big_endian( &time, 4, &cs2[4] );	// time2
		memcpy( &cs2[8], &result[9], hsr_size );				// randomData
		connection_->Write( cs2 );

		// ---[ Receive S2 packet ]----------
		connection_->Read( hs2_size, ref new ConnectionCallbackHandler( [=]( std::vector<uint8> result )
		{
			// S2 --- time: uint32, time2: uint32, random_echo: 1528 bytes
			uint32 server_send_client_time( 0 );
			utility::convert_big_endian( &result[0], 4, &server_send_client_time );

			// check time and random_echo
			if( time != server_send_client_time || memcmp( &send_data[9], &result[8], hsr_size ) != 0 )
				throw ref new Platform::FailureException();

			callbackFunction();
		} ) );
	} ) );
}