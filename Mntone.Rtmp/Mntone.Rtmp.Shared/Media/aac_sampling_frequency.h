#pragma once

namespace mntone { namespace rtmp { namespace media {

	enum class aac_sampling_frequency: unsigned
	{
		f96000 = 0,
		f88200 = 1,
		f64000 = 2,
		f48000 = 3,
		f44100 = 4,
		f32000 = 5,
		f24000 = 6,
		f22050 = 7,
		f16000 = 8,
		f12000 = 9,
		f11025 = 10,
		f8000 = 11,
		f7350 = 12,
		other = 15,
	};

} } }