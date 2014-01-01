#include "pch.h"
#if !_DEBUG
#include <random>
#endif
#include "handshake.h"

using namespace Mntone::Rtmp;

const auto hs0Size = 1u;
const auto hsrSize = 1528u;
const auto hs1Size = 8u + hsrSize;
const auto hs2Size = 8u + hsrSize;

void mntone::rtmp::handshake( NetConnection^ self )
{
	std::vector<uint8> sendData( hs0Size + hs1Size );

	// ---[ Start handshake ]----------
	// C0 --- protcolVersion: uint8
	sendData[0] = 0x03; // default: 0x03: plain (or 0x06, 0x08, 0x09: encrypted)

	// C1 --- time: uint32, zero: uint32, randomData: 1528 bytes
	// [time]
	auto time = utility::hundred_nano_to_milli( utility::get_windows_time() - self->startTime_ ) + 1;
	utility::convert_big_endian( &time, 4, &sendData[1] );

	// [zero]
	memset( &sendData[5], 0x00, 4 );

	// [randomData_]
#if _DEBUG
	memset( &sendData[9], 0xff, hsrSize );
#else
	std::mt19937 engine;
	std::uniform_int_distribution<uint64_t> distribution( 0x0000000000000000, 0xffffffffffffffff );
	auto sptr = reinterpret_cast<uint64_t*>( sendData.data() + 9 );
	const auto eptr = sptr + hsrSize / 8u;
	for( auto ptr = sptr; ptr != eptr; ++ptr )
		*ptr = distribution( engine );
#endif

	self->connection_->Write( sendData );
	// ---[ Sent C0+C1 packet ]----------

	std::vector<uint8> receiveData( hs0Size + hs1Size );
	auto rptr = receiveData.data();
	self->connection_->Read( rptr, receiveData.size() );
	// ---[ Received S0+S1 packet ]----------

	// S0 --- protocolVersion: uint8
	if( receiveData[0] != 0x03 )
		throw ref new Platform::FailureException();

	// S1 --- time: uint32, zero: uint32, randomData: 1528 bytes

	// Create C2 data
	std::vector<uint8> cs2( hs2Size );
	memcpy( &cs2[0], rptr + 1, 4 );						// time
	utility::convert_big_endian( &time, 4, &cs2[4] );	// time2
	memcpy( &cs2[8], rptr + 9, hsrSize );				// randomData

	self->connection_->Write( cs2 );
	// ---[ Sent C2 packet ]----------

	self->connection_->Read( cs2 );
	// ---[ Received S2 packet ]----------

	// S2 --- time: uint32, time2: uint32, randomEcho: 1528 bytes
	uint32 serverSendClientTime( 0 );
	utility::convert_big_endian( &cs2[0], 4, &serverSendClientTime );

	// check time and randomEcho
	if( time != serverSendClientTime || memcmp( &sendData[9], &cs2[8], hsrSize ) != 0 )
		throw ref new Platform::FailureException();
}