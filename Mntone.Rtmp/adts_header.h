#pragma once
#include "aac_id.h"
#include "aac_protection_absent.h"
#include "aac_profile.h"
#include "aac_sampling_frequency.h"

namespace Mntone { namespace Rtmp {

	_declspec( align( 1 ) )
	class adts_header
	{
	public:
		adts_header( void ) :
			sync1( 0xff ),
			sync2( 0xf ),
			layer( 0 ),
			frame_length1( 0 ),
			frame_length2( 0 ),
			frame_length3( 0 ),
			adts_buffer_fullness1( 0x1f ),
			adts_buffer_fullness2( 0x3f ),
			no_rawData__blocks_in_frame( 0 )
		{ }

		uint8 get_channel_configuration( void ) const { return static_cast<uint8>( channel_configuration1 << 2 | channel_configuration2 ); }
		void set_channel_configuration( const uint8 value )
		{
			channel_configuration1 = ( value >> 2 ) & 0x01;
			channel_configuration2 = value & 0x03;
		}

		uint16 get_frame_length( void ) const { return static_cast<uint16>( frame_length1 << 11 | frame_length2 << 3 | frame_length3 ); }
		void set_frame_length( const uint16 value )
		{
			frame_length1 = ( value >> 11 ) & 0x03;
			frame_length2 = ( value >> 3 ) & 0xff;
			frame_length3 = value & 0x07;
		}

		uint32 get_sampling_frequency_as_uint( void )
		{
			uint32 sampling_frequency;
			switch( sampling_frequency_index )
			{
			case aac_sampling_frequency::asf_96000: sampling_frequency = 96000; break;
			case aac_sampling_frequency::asf_88200: sampling_frequency = 88200; break;
			case aac_sampling_frequency::asf_64000: sampling_frequency = 64000; break;
			case aac_sampling_frequency::asf_48000: sampling_frequency = 48000; break;
			case aac_sampling_frequency::asf_44100: sampling_frequency = 44100; break;
			case aac_sampling_frequency::asf_32000: sampling_frequency = 32000; break;
			case aac_sampling_frequency::asf_24000: sampling_frequency = 24000; break;
			case aac_sampling_frequency::asf_22050: sampling_frequency = 22050; break;
			case aac_sampling_frequency::asf_16000: sampling_frequency = 16000; break;
			case aac_sampling_frequency::asf_12000: sampling_frequency = 12000; break;
			case aac_sampling_frequency::asf_11025: sampling_frequency = 11025; break;
			case aac_sampling_frequency::asf_8000: sampling_frequency = 8000; break;
			default: throw ref new Platform::InvalidArgumentException();
			}
			return sampling_frequency;
		}

	private: unsigned sync1 : 8;

	public: aac_protection_absent protection_absent : 1;
	private: unsigned layer : 2;
	public: aac_id id : 1;
	private: unsigned sync2 : 4;

	private: unsigned channel_configuration1 : 1;
	public: unsigned private_bit : 1;
	public: aac_sampling_frequency sampling_frequency_index : 4;
	public: aac_profile profile : 2;

	private: unsigned frame_length1 : 2;
	public: unsigned copyright_identification_start : 1;
	public: unsigned copyright_identification_bit : 1;
	public: unsigned home : 1;
	public: unsigned copy : 1;
	private: unsigned channel_configuration2 : 2;

	private:
		unsigned frame_length2 : 8;

		unsigned adts_buffer_fullness1 : 5;
		unsigned frame_length3 : 3;

		unsigned no_rawData__blocks_in_frame : 2;
		unsigned adts_buffer_fullness2 : 6;
	};

} }