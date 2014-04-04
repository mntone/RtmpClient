#pragma once

namespace mntone { namespace rtmp {

	enum class limit_type: uint8
	{
		hard = 0,
		soft = 1,
		dynamic = 2,
	};

} }