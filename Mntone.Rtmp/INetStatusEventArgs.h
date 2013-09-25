#pragma once

namespace Mntone { namespace Rtmp {

	enum class NetStatusType;

	[Windows::Foundation::Metadata::WebHostHidden]
	public interface class INetStatusEventArgs
	{
		property NetStatusType Status
		{
			NetStatusType get( void );
		}
	};

} }