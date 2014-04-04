#pragma once
#include "flv_tag_type.h"
#include "flv_filter.h"

namespace mntone { namespace rtmp { namespace media {

	class flv_tag
	{
	public:
		flv_tag()
			: reserved_( 0 )
			, filter_( flv_filter::no_pre_processing )
		{ }

		flv_tag_type tag_type() const noexcept { return tag_type_; }
		void set_tag_type( flv_tag_type value ) noexcept { tag_type_ = std::move( value ); }

		flv_filter filter() const noexcept { return filter_; }
		void set_filter( flv_filter value ) noexcept { filter_ = std::move( value ); }

		uint32 data_size() const noexcept;
		void set_data_size( uint32 value );

		int32 timestamp() const noexcept;
		void set_timestamp( int32 value ) noexcept;

		uint32 stream_id() const noexcept;
		void set_stream_id( uint32 value );

	private:
		flv_tag_type tag_type_ : 5;
		flv_filter filter_ : 1;
		unsigned reserved_ : 2;

		uint8 data_size_[3];

		uint8 timestamp_[3];
		int8 timestamp_extended_;

		uint8 stream_id_[3];
	};

} } }