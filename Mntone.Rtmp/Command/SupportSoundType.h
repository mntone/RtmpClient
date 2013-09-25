#pragma once

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::WebHostHidden]
	[Platform::Metadata::Flags]
	public enum class SupportSoundType: uint32
	{
		None = 0x0001,
		Adpcm = 0x0002,
		Mp3 = 0x0004,
		//Intel		= 0x0008,
		//Unused	= 0x0010,
		Nelly8 = 0x0020,
		Nelly = 0x0040,
		G771a = 0x0080,
		G771u = 0x0100,
		Nelly16 = 0x0200,
		Aac = 0x0400,
		Speex = 0x0800,
		AllAudio = 0x0fff,
	};

} } }