#include "pch.h"
#include "NetStreamAudioStartedEventArgs.h"

using namespace Mntone::Rtmp;

NetStreamAudioStartedEventArgs::NetStreamAudioStartedEventArgs( Media::AudioInfo^ info )
	: Info_( info )
{ }