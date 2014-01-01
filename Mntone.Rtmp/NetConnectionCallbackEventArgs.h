#pragma once

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetConnectionCallbackEventArgs sealed
	{
	internal:
		NetConnectionCallbackEventArgs( Platform::String^ commandName, Mntone::Data::Amf::IAmfValue^ response );

	public:
		property Platform::String^ CommandName
		{
			Platform::String^ get() { return CommandName_; }
		}
		property Mntone::Data::Amf::IAmfValue^ Response
		{
			Mntone::Data::Amf::IAmfValue^ get() { return Response_; }
		}

	private:
		Platform::String^ CommandName_;
		Mntone::Data::Amf::IAmfValue^ Response_;
	};

} }