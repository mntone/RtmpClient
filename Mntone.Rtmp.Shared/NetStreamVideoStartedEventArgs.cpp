#include "pch.h"
#include "NetStreamVideoStartedEventArgs.h"

using namespace Mntone::Rtmp;

NetStreamVideoStartedEventArgs::NetStreamVideoStartedEventArgs( bool videoOnly, Media::VideoInfo^ info )
	: VideoOnly_( videoOnly )
	, Info_( info )
{ }