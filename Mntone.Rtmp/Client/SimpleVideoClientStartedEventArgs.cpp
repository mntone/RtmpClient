#include "pch.h"
#include "SimpleVideoClientStartedEventArgs.h"

using namespace Mntone::Rtmp::Client;

SimpleVideoClientStartedEventArgs::SimpleVideoClientStartedEventArgs( Windows::Media::Core::MediaStreamSource^ mediaStreamSource ) :
	MediaStreamSource_( mediaStreamSource )
{ }