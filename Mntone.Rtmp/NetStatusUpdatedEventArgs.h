#pragma once
#include "NetStatusCodeType.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStatusUpdatedEventArgs sealed
	{
	internal:
		NetStatusUpdatedEventArgs( NetStatusCodeType netStatusCode );

	public:
		property NetStatusCodeType NetStatusCode
		{
			NetStatusCodeType get( void ) { return _NetStatusCode; }
		}

	private:
		NetStatusCodeType _NetStatusCode;
	};

} }