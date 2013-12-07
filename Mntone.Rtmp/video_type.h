#pragma once

namespace Mntone { namespace Rtmp {

	enum video_type: uint8
	{
		vt_keyframe = 1,
		vt_interframe = 2,
		vt_disposable_interframe = 3,
		vt_generated_keyframe = 4,
		vt_videoInfo__or_command_frame = 5,
	};

} }