#include "pch.h"
#include "NetStatusUpdatedEventArgs.h"

using namespace Mntone::Rtmp;

NetStatusUpdatedEventArgs::NetStatusUpdatedEventArgs( NetStatusCodeType netStatusCode ) :
_NetStatusCode( netStatusCode )
{ }