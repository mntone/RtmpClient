#pragma once
#include "sound_format.h"
#include "sound_rate.h"
#include "sound_size.h"
#include "sound_type.h"

namespace mntone { namespace rtmp { namespace media {

	struct sound_info
	{
		sound_type type : 1;
		sound_size size : 1;
		sound_rate rate : 2;
		sound_format format : 4;
	};

} } }