#pragma once
#include "type_id_type.h"

namespace Mntone { namespace Rtmp {

	struct rtmp_packet
	{
		uint16 ChunkStreamId;
		int64 Timestamp;
		uint32 Length;
		type_id_type TypeId;
		uint32 StreamId;
	};

} }
