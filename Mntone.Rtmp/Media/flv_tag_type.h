#pragma once

namespace mntone { namespace rtmp { namespace media {

	enum class flv_tag_type: uint8
	{
		audio = 8,
		video = 9,
		script_data = 18,
	};

} } }