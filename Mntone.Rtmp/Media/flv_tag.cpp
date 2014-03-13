#include "pch.h"
#include "flv_tag.h"
#include "utility.h"

using namespace mntone::rtmp::media;

uint32 flv_tag::data_size() const noexcept
{
	uint32 ret( 0 );
	utility::convert_big_endian( data_size_, 3, &ret );
	return ret;
}
void flv_tag::set_data_size( uint32 value )
{
	if( value > 281474976710655 )
	{
		throw ref new Platform::InvalidArgumentException();
	}

	utility::convert_big_endian( &value, 3, &data_size_[0] );
}

int32 flv_tag::timestamp() const noexcept
{
	int32 ret( 0 );
	utility::convert_big_endian( timestamp_, 3, &ret );
	ret |= timestamp_extended_ << 24;
	return ret;
}
void flv_tag::set_timestamp( int32 value ) noexcept
{
	utility::convert_big_endian( &value, 3, &timestamp_[0] );
	timestamp_extended_ = value >> 24;
}

uint32 flv_tag::stream_id() const noexcept
{
	uint32 ret( 0 );
	utility::convert_big_endian( stream_id_, 3, &ret );
	return ret;
}
void flv_tag::set_stream_id( uint32 value )
{
	if( value > 281474976710655 )
	{
		throw ref new Platform::InvalidArgumentException();
	}

	utility::convert_big_endian( &value, 3, &stream_id_[0] );
}