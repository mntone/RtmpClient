#pragma once
#include <memory>
#include "type_id_type.h"

namespace mntone { namespace rtmp {

	struct rtmp_header
	{
		explicit rtmp_header( uint16 chunk_stream_id )
			: chunk_stream_id( chunk_stream_id )
			, timestamp( 0 )
			, timestamp_delta( 0 )
			, length( 0 )
			, stream_id( 0 )
		{ }

		uint16 chunk_stream_id;
		int64 timestamp, timestamp_delta;
		uint32 length;
		type_id_type type_id;
		uint32 stream_id;
	};

} }
