#pragma once
#include "VideoInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamVideoStartedEventArgs sealed
	{
	internal:
		NetStreamVideoStartedEventArgs( VideoInfo^ info );

	public:
		property VideoInfo^ Info
		{
			VideoInfo^ get( void ) { return _Info; }
		}

	private:
		VideoInfo^ _Info;
	};

} }