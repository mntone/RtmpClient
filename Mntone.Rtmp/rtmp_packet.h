#pragma once
#include "type_id_type.h"

namespace mntone { namespace rtmp {

	struct rtmp_packet
	{
		uint16 chunk_stream_id_;
		int64 timestamp_, timestamp_delta_;
		uint32 length_;
		type_id_type type_id_;
		uint32 stream_id_;
	};

} }
