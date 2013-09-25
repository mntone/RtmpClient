#pragma once
#include "NetStatusType.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStatusUpdatedEventArgs sealed
	{
	internal:
		NetStatusUpdatedEventArgs( NetStatusType netStatus );

	public:
		property NetStatusType NetStatus
		{
			NetStatusType get( void ) { return _NetStatus; }
		}

	private:
		NetStatusType _NetStatus;
	};

} }