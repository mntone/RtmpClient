#pragma once
#include "IRtmpCommand.h"
#include "SupportSoundType.h"
#include "SupportVideoType.h"
#include "SupportVideoFunctionType.h"

namespace Mntone { namespace Rtmp { namespace Command {

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WIN8 )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::Both )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetConnectionConnectCommand sealed:
		public IRtmpCommand
	{
	public:
		NetConnectionConnectCommand( Platform::String^ app );

		virtual Mntone::Data::Amf::AmfArray^ Commandify();

		// IStringable
		virtual Platform::String^ ToString() override sealed;

	public:
		property Platform::String^ Type
		{
			virtual Platform::String^ get() { return "connect"; }
		}
		property Platform::String^ App
		{
			Platform::String^ get() { return App_; }
			void set( Platform::String^ value ) { App_ = value; }
		}
		property Platform::String^ FlashVersion
		{
			Platform::String^ get() { return FlashVersion_; }
			void set( Platform::String^ value ) { FlashVersion_ = value; }
		}
		property Platform::String^ SwfUrl
		{
			Platform::String^ get() { return SwfUrl_; }
			void set( Platform::String^ value ) { SwfUrl_ = value; }
		}
		property Platform::String^ TcUrl
		{
			Platform::String^ get() { return TcUrl_; }
			void set( Platform::String^ value ) { TcUrl_ = value; }
		}
		property bool Fpad
		{
			bool get() { return Fpad_; }
			void set( bool value ) { Fpad_ = value; }
		}
		property SupportSoundType AudioCodecs
		{
			SupportSoundType get() { return AudioCodecs_; }
			void set( SupportSoundType value ) { AudioCodecs_ = value; }
		}
		property SupportVideoType VideoCodecs
		{
			SupportVideoType get() { return VideoCodecs_; }
			void set( SupportVideoType value ) { VideoCodecs_ = value; }
		}
		property SupportVideoFunctionType VideoFunction
		{
			SupportVideoFunctionType get() { return VideoFunction_; }
			void set( SupportVideoFunctionType value ) { VideoFunction_ = value; }
		}
		property Platform::String^ PageUrl
		{
			Platform::String^ get() { return PageUrl_; }
			void set( Platform::String^ value ) { PageUrl_ = value; }
		}
		property Mntone::Data::Amf::AmfEncodingType ObjectEncoding
		{
			Mntone::Data::Amf::AmfEncodingType get() { return ObjectEncoding_; }
			//void set( Mntone::Data::Amf::AmfEncodingType value ) { ObjectEncoding_ = value; }
		}
		property Mntone::Data::Amf::IAmfValue^ OptionalUserArguments
		{
			Mntone::Data::Amf::IAmfValue^ get() { return OptionalUserArguments_; }
			void set( Mntone::Data::Amf::IAmfValue^ value ) { OptionalUserArguments_ = value; }
		}

	private:
		Platform::String^ App_;
		Platform::String^ FlashVersion_;
		Platform::String^ SwfUrl_;
		Platform::String^ TcUrl_;
		bool Fpad_;
		SupportSoundType AudioCodecs_;
		SupportVideoType VideoCodecs_;
		SupportVideoFunctionType VideoFunction_;
		Platform::String^ PageUrl_;
		Mntone::Data::Amf::AmfEncodingType ObjectEncoding_;
		Mntone::Data::Amf::IAmfValue^ OptionalUserArguments_;
	};

} } }