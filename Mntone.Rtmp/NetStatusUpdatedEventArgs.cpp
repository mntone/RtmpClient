#include "pch.h"
#include "NetStatusUpdatedEventArgs.h"

using namespace Mntone::Rtmp;

NetStatusUpdatedEventArgs::NetStatusUpdatedEventArgs( NetStatusType netStatus ) :
	_NetStatus( netStatus )
{ }