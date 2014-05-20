#pragma once
#include "IRtmpCommand.h"

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class RawRtmpCommand sealed
		: public IRtmpCommand
	{
	public:
		RawRtmpCommand( Platform::String^ type, Mntone::Data::Amf::AmfArray^ command );

		virtual Mntone::Data::Amf::AmfArray^ Commandify();

	public:
		property Platform::String^ Type
		{
			virtual Platform::String^ get() { return Type_; }
		}

	private:
		Platform::String^ Type_;
		Mntone::Data::Amf::AmfArray^ command_;
	};

} } }