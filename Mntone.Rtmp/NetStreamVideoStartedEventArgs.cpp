#include "pch.h"
#include "NetStreamVideoStartedEventArgs.h"

using namespace Mntone::Rtmp;

NetStreamVideoStartedEventArgs::NetStreamVideoStartedEventArgs( VideoInfo^ info ) :
	Info_( info )
{ }