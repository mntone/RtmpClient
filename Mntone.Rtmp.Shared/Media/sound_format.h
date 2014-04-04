#pragma once

namespace mntone { namespace rtmp { namespace media {

	enum class sound_format: uint8
	{
		linear_pcm = 0,
		adaptive_differential_pcm = 1,
		mp3 = 2,
		linear_pcm_little_endian = 3,
		nellymoser_16khz_mono = 4,
		nellymoser_8khz_mono = 5,
		nellymoser = 6,
		g711_alaw_logarithmic_pcm = 7,
		g711_mulaw_logarithmic_pcm = 8,
		// reserved = 9,
		aac = 10,
		speex = 11,
		mp3_8khz = 14,
		device_specific_sound = 15,
	};

} } }