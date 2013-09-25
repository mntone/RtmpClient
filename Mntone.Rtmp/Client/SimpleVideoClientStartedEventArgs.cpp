#include "pch.h"
#include "SimpleVideoClientStartedEventArgs.h"

namespace Mntone { namespace Rtmp { namespace Client {

	SimpleVideoClientStartedEventArgs::SimpleVideoClientStartedEventArgs( Windows::Media::Core::MediaStreamSource^ mediaStreamSource ):
		_MediaStreamSource( mediaStreamSource )
	{ }

} } }