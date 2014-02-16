#pragma once
#include "aac_id.h"
#include "aac_protection_absent.h"
#include "aac_profile.h"
#include "aac_sampling_frequency.h"

namespace mntone { namespace rtmp {

	class adts_header
	{
	public:
		adts_header() :
			sync1_( 0xff ),
			sync2_( 0xf ),
			layer_( 0 ),
			frame_length1_( 0 ),
			frame_length2_( 0 ),
			frame_length3_( 0 ),
			adts_buffer_fullness1( 0x1f ),
			adts_buffer_fullness2_( 0x3f ),
			no_raw_data_blocks_in_frame_( 0 )
		{ }

		mntone::rtmp::aac_id id() const noexcept { return id_; }
		void set_id( mntone::rtmp::aac_id value ) noexcept { id_ = std::move( value ); }

		uint8 layer() const noexcept { return layer_; }
		void set_layer( uint8 value ) noexcept { layer_ = std::move( value ); }

		mntone::rtmp::aac_protection_absent protection_absent() const noexcept { return protection_absent_; }
		void set_protection_absent( mntone::rtmp::aac_protection_absent value ) noexcept { protection_absent_ = std::move( value ); }

		mntone::rtmp::aac_profile profile() const noexcept { return profile_; }
		void set_profile( mntone::rtmp::aac_profile value ) noexcept { profile_ = std::move( value ); }

		mntone::rtmp::aac_sampling_frequency sampling_frequency_index() const noexcept { return sampling_frequency_index_; }
		void set_sampling_frequency_index( mntone::rtmp::aac_sampling_frequency value ) noexcept { sampling_frequency_index_ = std::move( value ); }

		uint32 sampling_frequency() const
		{
			uint32 sampling_frequency;
			switch( sampling_frequency_index_ )
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
			case aac_sampling_frequency::asf_7350: sampling_frequency = 7350; break;
			default: throw ref new Platform::InvalidArgumentException();
			}
			return sampling_frequency;
		}
		void set_sampling_frequency( uint32 sampling_frequency )
		{
			switch( sampling_frequency )
			{
			case 96000: sampling_frequency_index_ = aac_sampling_frequency::asf_96000; break;
			case 88200: sampling_frequency_index_ = aac_sampling_frequency::asf_88200; break;
			case 64000: sampling_frequency_index_ = aac_sampling_frequency::asf_64000; break;
			case 48000: sampling_frequency_index_ = aac_sampling_frequency::asf_48000; break;
			case 44100: sampling_frequency_index_ = aac_sampling_frequency::asf_44100; break;
			case 32000: sampling_frequency_index_ = aac_sampling_frequency::asf_32000; break;
			case 24000: sampling_frequency_index_ = aac_sampling_frequency::asf_24000; break;
			case 22050: sampling_frequency_index_ = aac_sampling_frequency::asf_22050; break;
			case 16000: sampling_frequency_index_ = aac_sampling_frequency::asf_16000; break;
			case 12000: sampling_frequency_index_ = aac_sampling_frequency::asf_12000; break;
			case 11025: sampling_frequency_index_ = aac_sampling_frequency::asf_11025; break;
			case 8000: sampling_frequency_index_ = aac_sampling_frequency::asf_8000; break;
			case 7350: sampling_frequency_index_ = aac_sampling_frequency::asf_7350; break;
			default: throw ref new Platform::InvalidArgumentException();
			}
		}

		bool private_bit() const noexcept { return private_bit_ != 0; }
		void set_private_bit( bool value ) noexcept { private_bit_ = value ? 1 : 0; }

		uint8 channel_configuration() const noexcept { return static_cast<uint8>( channel_configuration1_ << 2 | channel_configuration2_ ); }
		void set_channel_configuration( uint8 value )
		{
			channel_configuration1_ = ( value >> 2 ) & 0x01;
			channel_configuration2_ = value & 0x03;
		}

		bool copy() const noexcept { return copy_ != 0; }
		void set_copy( bool value ) noexcept { copy_ = value ? 1 : 0; }

		bool home() const noexcept { return home_ != 0; }
		void set_home( bool value ) noexcept { home_ = value ? 1 : 0; }

		bool copyright_identification_bit() const noexcept { return copyright_identification_bit_ != 0; }
		void set_copyright_identification_bit( bool value ) noexcept { copyright_identification_bit_ = value ? 1 : 0; }

		bool copyright_identification_start() const noexcept { return copyright_identification_start_ != 0; }
		void set_copyright_identification_start( bool value ) noexcept { copyright_identification_start_ = value ? 1 : 0; }

		uint16 frame_length() const noexcept { return static_cast<uint16>( frame_length1_ << 11 | frame_length2_ << 3 | frame_length3_ ); }
		void set_frame_length( uint16 value )
		{
			frame_length1_ = ( value >> 11 ) & 0x03;
			frame_length2_ = ( value >> 3 ) & 0xff;
			frame_length3_ = value & 0x07;
		}

	private:
		unsigned sync1_ : 8;

		mntone::rtmp::aac_protection_absent protection_absent_ : 1;
		unsigned layer_ : 2;
		mntone::rtmp::aac_id id_ : 1;
		unsigned sync2_ : 4;

		unsigned channel_configuration1_ : 1;
		unsigned private_bit_ : 1;
		mntone::rtmp::aac_sampling_frequency sampling_frequency_index_ : 4;
		mntone::rtmp::aac_profile profile_ : 2;

		unsigned frame_length1_ : 2;
		unsigned copyright_identification_start_ : 1;
		unsigned copyright_identification_bit_ : 1;
		unsigned home_ : 1;
		unsigned copy_ : 1;
		unsigned channel_configuration2_ : 2;

		unsigned frame_length2_ : 8;

		unsigned adts_buffer_fullness1 : 5;
		unsigned frame_length3_ : 3;

		unsigned no_raw_data_blocks_in_frame_ : 2;
		unsigned adts_buffer_fullness2_ : 6;
	};

} }