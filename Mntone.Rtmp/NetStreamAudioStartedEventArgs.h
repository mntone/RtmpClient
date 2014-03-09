#pragma once
#include "Media/AudioInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamAudioStartedEventArgs sealed
	{
	internal:
		NetStreamAudioStartedEventArgs( bool audioOnly, Media::AudioInfo^ info );

	public:
		property bool AudioOnly
		{
			bool get() { return AudioOnly_; }
		}
		property Media::AudioInfo^ Info
		{
			Media::AudioInfo^ get() { return Info_; }
		}

	private:
		bool AudioOnly_;
		Media::AudioInfo^ Info_;
	};

} }