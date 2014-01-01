#include "pch.h"
#include "NetConnectionCallbackEventArgs.h"

using namespace Mntone::Rtmp;

NetConnectionCallbackEventArgs::NetConnectionCallbackEventArgs( Platform::String^ commandName, Mntone::Data::Amf::IAmfValue^ response )
	: CommandName_( commandName )
	, Response_( response )
{ }