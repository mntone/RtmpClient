#include "pch.h"
#include "NetStatusUpdatedEventArgs.h"

namespace Mntone { namespace Rtmp {

	NetStatusUpdatedEventArgs::NetStatusUpdatedEventArgs( NetStatusType netStatus ) :
		_NetStatus( netStatus )
	{ }

} }