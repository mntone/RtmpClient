#pragma once

namespace mntone { namespace rtmp {

	enum limit_type: uint8
	{
		limit_hard = 0,
		limit_soft = 1,
		limit_dynamic = 2,
	};

} }