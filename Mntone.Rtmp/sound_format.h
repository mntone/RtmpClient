#pragma once

namespace mntone { namespace rtmp {

	enum sound_format: uint8
	{
		sf_lpcm = 0,
		sf_adpcm = 1,
		sf_mp3 = 2,
		sf_lpcm_little_endian = 3,
		sf_nellymoser16khz_mono = 4,
		sf_nellymoser8khz_mono = 5,
		sf_nellymoser = 6,
		sf_g711_alaw_logarithmic_pcm = 7,
		sf_g711_mulaw_logarithmic_pcm = 8,
		// sf_reserved = 9,
		sf_aac = 10,
		sf_speex = 11,
		sf_mp38khz = 14,
		sf_device_specific_sound = 15,
	};

} }