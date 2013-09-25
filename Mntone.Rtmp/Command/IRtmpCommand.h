#pragma once

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::WebHostHidden]
	public interface class IRtmpCommand
	{
		property Platform::String^ Type
		{
			Platform::String^ get();
		}

		Mntone::Data::Amf::AmfArray^ Commandify( void );
	};

} } }