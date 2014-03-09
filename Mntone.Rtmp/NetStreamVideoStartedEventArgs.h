#pragma once
#include "Media/VideoInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamVideoStartedEventArgs sealed
	{
	internal:
		NetStreamVideoStartedEventArgs( bool videoOnly, Media::VideoInfo^ info );

	public:
		property bool VideoOnly
		{
			bool get() { return VideoOnly_; }
		}
		property Media::VideoInfo^ Info
		{
			Media::VideoInfo^ get() { return Info_; }
		}

	private:
		bool VideoOnly_;
		Media::VideoInfo^ Info_;
	};

} }