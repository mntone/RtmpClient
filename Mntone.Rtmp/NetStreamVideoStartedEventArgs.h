#pragma once
#include "Media/VideoInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamVideoStartedEventArgs sealed
	{
	internal:
		NetStreamVideoStartedEventArgs( Media::VideoInfo^ info );

	public:
		property Media::VideoInfo^ Info
		{
			Media::VideoInfo^ get() { return Info_; }
		}

	private:
		Media::VideoInfo^ Info_;
	};

} }