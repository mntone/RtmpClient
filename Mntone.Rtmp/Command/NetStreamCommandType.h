#pragma once

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::WebHostHidden]
	public enum class NetStreamCommandType
	{
		Play = 0,
		Play2,
		DeleteStream,
		CloseStream, 
		ReceiveAudio, 
		ReceiveVideo,
		Publish, 
		Seek,
		Pause,
	};

} } }