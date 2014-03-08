#include "pch.h"
#include "NetStreamVideoStartedEventArgs.h"

using namespace Mntone::Rtmp;

NetStreamVideoStartedEventArgs::NetStreamVideoStartedEventArgs( Media::VideoInfo^ info )
	: Info_( info )
{ }