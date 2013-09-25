#pragma once

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::WebHostHidden]
	public enum class NetConnectionCommandType
	{
		Connect,
		Call,
		//Close,
		CreateStream = 3,
	};

} } }