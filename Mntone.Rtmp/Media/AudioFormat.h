#pragma once

namespace Mntone { namespace Rtmp { namespace Media {

	[Windows::Foundation::Metadata::WebHostHidden]
	public enum class AudioFormat
	{
		Lpcm = 1,
		Adpcm,
		Mp3,
		LpcmLe,
		Nellymoser,
		G711Alaw,
		G711Mulaw,
		Aac,
		Speex,
	};

} } }