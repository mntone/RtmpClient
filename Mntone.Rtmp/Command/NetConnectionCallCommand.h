#pragma once
#include "IRtmpCommand.h"

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WIN8 )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::Both )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetConnectionCallCommand sealed
		: public IRtmpCommand
	{
	public:
		NetConnectionCallCommand( Platform::String^ commandName );

		virtual Mntone::Data::Amf::AmfArray^ Commandify();

		// IStringable
		virtual Platform::String^ ToString() override sealed;

	public:
		property Platform::String^ Type
		{
			virtual Platform::String^ get() { return "call"; }
		}
		property Platform::String^ CommandName
		{
			Platform::String^ get() { return CommandName_; }
			void set( Platform::String^ value ) { CommandName_ = value; }
		}
		property Mntone::Data::Amf::IAmfValue^ CommandObject
		{
			Mntone::Data::Amf::IAmfValue^ get() { return CommandObject_; }
			void set( Mntone::Data::Amf::IAmfValue^ value ) { CommandObject_ = value; }
		}
		property Windows::Foundation::Collections::IVectorView<Mntone::Data::Amf::IAmfValue^>^ OptionalArguments
		{
			Windows::Foundation::Collections::IVectorView<Mntone::Data::Amf::IAmfValue^>^ get() { return OptionalArguments_; }
			void set( Windows::Foundation::Collections::IVectorView<Mntone::Data::Amf::IAmfValue^>^ value ) { OptionalArguments_ = value; }
		}

	internal:
		property uint32 TransactionId
		{
			uint32 get() { return TransactionId_; }
			void set( uint32 value ) { TransactionId_ = value; }
		}

	private:
		Platform::String^ CommandName_;
		uint32 TransactionId_;
		Mntone::Data::Amf::IAmfValue^ CommandObject_;
		Windows::Foundation::Collections::IVectorView<Mntone::Data::Amf::IAmfValue^>^ OptionalArguments_;
	};

} } }