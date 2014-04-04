#pragma once

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::WebHostHidden]
	[Platform::Metadata::Flags]
	public enum class SupportVideoType: uint32
	{
		//Unused = 0x01,
		//Jpeg = 0x02,
		Sorenson = 0x04,
		Homebrew = 0x08,
		Vp6 = 0x10,
		Vp6alpha = 0x20,
		Homebrewv = 0x40,
		H264 = 0x80,
		AllVideo = 0xff,
	};

} } }