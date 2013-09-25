#pragma once
#include "NetConnection.h"

namespace Mntone { namespace Rtmp {

	ref class Handshaker sealed
	{
	internal:
		static void Handshake( NetConnection^ connection );
	};

} }
