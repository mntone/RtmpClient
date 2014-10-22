#include "pch.h"
#include "audio_specific_config.h"
#include "utility/bit_iterator.h"

using namespace mntone::rtmp::media;

audio_specific_config::audio_specific_config( const uint8* data, const size_t size )
{
	load( data, size );
}

void audio_specific_config::load( const uint8* data, const size_t size )
{
	utility::bit_iterator itr(
	const_cast<const utility::bit_iterator::bit_array_pointer>( data ),
	static_cast<utility::bit_iterator::bit_array_length>( size ) );

	object_type_ = itr.get_and_move_length<int8>( 5 );
	if( object_type_ == 0x1f )
	{
		object_type_ = 32 + itr.get_and_move_length<int8>( 6 );
	}

	const auto sampling_frequency_index = static_cast<aac_sampling_frequency>( itr.get_and_move_length<int32>( 4 ) );
	if( sampling_frequency_index != aac_sampling_frequency::other )
	{
		sampling_frequency_ = get_sampling_frequency( sampling_frequency_index );
	}
	else
	{
		sampling_frequency_ = itr.get_and_move_length<int32>( 24 );
	}

	channel_configuration_ = itr.get_and_move_length<int8>( 4 );

	if( object_type_ == 5 || object_type_ == 29 )
	{
		is_sbr_ = true;
		if( object_type_ == 29 )
		{
			is_ps_ = true;
		}

		const auto extension_sampling_frequency_index = static_cast<aac_sampling_frequency>( itr.get_and_move_length<int32>( 4 ) );
		if( extension_sampling_frequency_index != aac_sampling_frequency::other )
		{
			extension_sampling_frequency_ = get_sampling_frequency( extension_sampling_frequency_index );
		}
		else
		{
			extension_sampling_frequency_ = itr.get_and_move_length<int32>( 24 );
		}

		extension_channel_configuration_ = itr.get_and_move_length<int8>( 4 );
	}
	else
	{
		extension_object_type_ = 0;
	}
}

int32 audio_specific_config::get_sampling_frequency( aac_sampling_frequency value ) const
{
	int32 ret;
	switch( value )
	{
	case aac_sampling_frequency::f96000: ret = 96000; break;
	case aac_sampling_frequency::f88200: ret = 88200; break;
	case aac_sampling_frequency::f64000: ret = 64000; break;
	case aac_sampling_frequency::f48000: ret = 48000; break;
	case aac_sampling_frequency::f44100: ret = 44100; break;
	case aac_sampling_frequency::f32000: ret = 32000; break;
	case aac_sampling_frequency::f24000: ret = 24000; break;
	case aac_sampling_frequency::f22050: ret = 22050; break;
	case aac_sampling_frequency::f16000: ret = 16000; break;
	case aac_sampling_frequency::f12000: ret = 12000; break;
	case aac_sampling_frequency::f11025: ret = 11025; break;
	case aac_sampling_frequency::f8000: ret = 8000; break;
	case aac_sampling_frequency::f7350: ret = 7350; break;
	default: throw ref new Platform::InvalidArgumentException();
	}
	return ret;
}