#pragma once
#include <memory>
#include "type_id_type.h"

namespace mntone { namespace rtmp {

	struct rtmp_packet
	{
		rtmp_packet( uint16 chunk_stream_id )
			: chunk_stream_id_( chunk_stream_id )
			, timestamp_delta_( 0 )
			, length_( 0 )
			, temporary_length_( 0 )
		{ }

		uint16 chunk_stream_id_;
		int64 timestamp_, timestamp_delta_;
		uint32 length_;
		type_id_type type_id_;
		uint32 stream_id_;

		uint32 temporary_length_;
		std::shared_ptr<std::vector<uint8>> body_;
	};

} }
