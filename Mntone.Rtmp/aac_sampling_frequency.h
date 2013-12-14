#pragma once

namespace mntone { namespace rtmp {

	enum aac_sampling_frequency: unsigned
	{
		asf_96000 = 0,
		asf_88200 = 1,
		asf_64000 = 2,
		asf_48000 = 3,
		asf_44100 = 4,
		asf_32000 = 5,
		asf_24000 = 6,
		asf_22050 = 7,
		asf_16000 = 8,
		asf_12000 = 9,
		asf_11025 = 10,
		asf_8000 = 11,
	};

} }