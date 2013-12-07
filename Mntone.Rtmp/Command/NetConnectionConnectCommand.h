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

		virtual Mntone::Data::Amf::AmfArray^ Commandify( void );

		// IStringable
		virtual Platform::String^ ToString( void ) override sealed;

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
			Platform::String^ get() { return _FlashVersion; }
			void set( Platform::String^ value ) { _FlashVersion = value; }
		}
		property Platform::String^ SwfUrl
		{
			Platform::String^ get() { return _SwfUrl; }
			void set( Platform::String^ value ) { _SwfUrl = value; }
		}
		property Platform::String^ TcUrl
		{
			Platform::String^ get() { return _TcUrl; }
			void set( Platform::String^ value ) { _TcUrl = value; }
		}
		property bool Fpad
		{
			bool get() { return _Fpad; }
			void set( bool value ) { _Fpad = value; }
		}
		property SupportSoundType AudioCodecs
		{
			SupportSoundType get() { return _AudioCodecs; }
			void set( SupportSoundType value ) { _AudioCodecs = value; }
		}
		property SupportVideoType VideoCodecs
		{
			SupportVideoType get() { return _VideoCodecs; }
			void set( SupportVideoType value ) { _VideoCodecs = value; }
		}
		property SupportVideoFunctionType VideoFunction
		{
			SupportVideoFunctionType get() { return _VideoFunction; }
			void set( SupportVideoFunctionType value ) { _VideoFunction = value; }
		}
		property Platform::String^ PageUrl
		{
			Platform::String^ get() { return _PageUrl; }
			void set( Platform::String^ value ) { _PageUrl = value; }
		}
		property Mntone::Data::Amf::AmfEncodingType ObjectEncoding
		{
			Mntone::Data::Amf::AmfEncodingType get() { return _ObjectEncoding; }
			//void set( Mntone::Data::Amf::AmfEncodingType value ) { _ObjectEncoding = value; }
		}
		property Mntone::Data::Amf::IAmfValue^ OptionalUserArguments
		{
			Mntone::Data::Amf::IAmfValue^ get() { return _OptionalUserArguments; }
			void set( Mntone::Data::Amf::IAmfValue^ value ) { _OptionalUserArguments = value; }
		}

	private:
		Platform::String^ App_;
		Platform::String^ _FlashVersion;
		Platform::String^ _SwfUrl;
		Platform::String^ _TcUrl;
		bool _Fpad;
		SupportSoundType _AudioCodecs;
		SupportVideoType _VideoCodecs;
		SupportVideoFunctionType _VideoFunction;
		Platform::String^ _PageUrl;
		Mntone::Data::Amf::AmfEncodingType _ObjectEncoding;
		Mntone::Data::Amf::IAmfValue^ _OptionalUserArguments;
	};

} } }