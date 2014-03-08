#pragma once

namespace mntone { namespace rtmp { namespace media {

	enum class video_type: uint8
	{
		keyframe = 1,
		interframe = 2,
		disposable_interframe = 3,
		generated_keyframe = 4,
		video_info_or_command_frame = 5,
	};

} } }