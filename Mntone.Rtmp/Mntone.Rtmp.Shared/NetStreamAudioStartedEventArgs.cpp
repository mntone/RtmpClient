#include "pch.h"
#include "NetStreamAudioStartedEventArgs.h"

using namespace Mntone::Rtmp;

NetStreamAudioStartedEventArgs::NetStreamAudioStartedEventArgs( bool audioOnly, Media::AudioInfo^ info )
	: AudioOnly_( audioOnly )
	, Info_( info )
{ }