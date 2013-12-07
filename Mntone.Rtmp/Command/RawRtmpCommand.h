#pragma once
#include "IRtmpCommand.h"

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WIN8 )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::Both )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class RawRtmpCommand sealed:
		public IRtmpCommand
	{
	public:
		RawRtmpCommand( Platform::String^ type, Mntone::Data::Amf::AmfArray^ command );

		virtual Mntone::Data::Amf::AmfArray^ Commandify();

	public:
		property Platform::String^ Type
		{
			virtual Platform::String^ get() { return _Type; }
		}

	private:
		Platform::String^ _Type;
		Mntone::Data::Amf::AmfArray^ _command;
	};

} } }