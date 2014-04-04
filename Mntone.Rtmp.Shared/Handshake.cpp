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

	// C0 --- protcol_version: uint8
	send_data[0] = 0x03; // default: 0x03: plain (or 0x06, 0x08, 0x09: encrypted)

	// C1 --- time: uint32, zero: uint32, random_data: 1528 bytes
	auto c1_time = utility::hundred_nano_to_milli( utility::get_windows_time() - startTime_ ) + 1;
	utility::convert_big_endian( &c1_time, 4, &send_data[1] );	// time
	std::fill_n( send_data.begin() + 5, 4, 0x00 );				// zero

	// random_data
#if NDEBUG
	std::mt19937 engine;
	std::uniform_int_distribution<uint64_t> distribution( 0x0000000000000000, 0xffffffffffffffff );
	auto sptr = reinterpret_cast<uint64_t*>( &send_data[9] );
	const auto eptr = sptr + hsr_size / 8u;
	for( auto ptr = sptr; ptr != eptr; ++ptr )
	{
		*ptr = distribution( engine );
	}
#endif

	create_task( connection_->Write( send_data.data(), send_data.size() ) ).then( [this, c1_time, send_data, callbackFunction]
	{
		// ---[ Receive S0+S1 packet ]----------
		connection_->Read( hs0_size + hs1_size, ref new ConnectionCallbackHandler( [this, c1_time, send_data, callbackFunction]( Windows::Storage::Streams::IBuffer^ result )
		{
			auto reader = Windows::Storage::Streams::DataReader::FromBuffer( result );
			reader->ByteOrder = Windows::Storage::Streams::ByteOrder::BigEndian;

			// S0 --- protocol_version: uint8
			if( reader->ReadByte() != 0x03 )
			{
				throw ref new Platform::FailureException();
			}

			// S1 --- time: uint32, zero: uint32, random_data: 1528 bytes
			const uint32 s1_time = reader->ReadUInt32();
			reader->ReadUInt32();

			// ---[ Send C2 packet ]----------
			std::vector<uint8> send_c2_data( hs2_size );
			utility::convert_big_endian( &s1_time, 4, &send_c2_data[0] );	// c2_time
			utility::convert_big_endian( &c1_time, 4, &send_c2_data[4] );	// c2_time2
			reader->ReadBytes( Platform::ArrayReference<uint8>( send_c2_data.data() + 8, static_cast<uint32>( send_c2_data.size() - 8 ) ) ); // random_data

			create_task( connection_->Write( send_c2_data.data(), send_c2_data.size() ) ).then( [this, c1_time, send_data, callbackFunction]
			{
				// ---[ Receive S2 packet ]----------
				connection_->Read( hs2_size, ref new ConnectionCallbackHandler( [c1_time, send_data, callbackFunction]( Windows::Storage::Streams::IBuffer^ result )
				{
					auto reader = Windows::Storage::Streams::DataReader::FromBuffer( result );
					reader->ByteOrder = Windows::Storage::Streams::ByteOrder::BigEndian;

					// S2 --- time: uint32, time2: uint32, random_data: 1528 bytes
					const uint32 s2_time = reader->ReadUInt32();
					/*const uint32 s2_time2 =*/ reader->ReadUInt32();

					auto s2_random_data = ref new Platform::Array<uint8>( hsr_size );
					reader->ReadBytes( s2_random_data );

					// check time and random_echo
					if( c1_time != s2_time || memcmp( &send_data[9], s2_random_data->Data, hsr_size ) != 0 )
					{
						throw ref new Platform::FailureException();
					}

					callbackFunction();
				} ) );
			} );
		} ) );
	} );
}