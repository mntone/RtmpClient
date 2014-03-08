#pragma once
#include "Media/AudioInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamAudioStartedEventArgs sealed
	{
	internal:
		NetStreamAudioStartedEventArgs( Media::AudioInfo^ info );

	public:
		property Media::AudioInfo^ Info
		{
			Media::AudioInfo^ get() { return Info_; }
		}

	private:
		Media::AudioInfo^ Info_;
	};

} }