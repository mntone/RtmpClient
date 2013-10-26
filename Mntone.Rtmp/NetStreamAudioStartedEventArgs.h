#pragma once
#include "AudioInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamAudioStartedEventArgs sealed
	{
	internal:
		NetStreamAudioStartedEventArgs( AudioInfo^ info );

	public:
		property AudioInfo^ Info
		{
			AudioInfo^ get( void ) { return _Info; }
		}

	private:
		AudioInfo^ _Info;
	};

} }