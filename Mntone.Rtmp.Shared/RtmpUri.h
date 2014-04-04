#pragma once
#include "RtmpScheme.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class RtmpUri sealed
	{
	public:
		RtmpUri();
		RtmpUri( Platform::String^ uri );
		[Windows::Foundation::Metadata::DefaultOverload]
		RtmpUri( Windows::Foundation::Uri^ uri );

		// IStringable
		virtual Platform::String^ ToString() override sealed;

	private:
		void ParseScheme( Platform::String^ schemeName );
		void ParsePort( int32 port );
		void ParsePath( Platform::String^ path );

	public:
		property RtmpScheme Scheme
		{
			RtmpScheme get() { return Scheme_; }
			void set( RtmpScheme value ) { Scheme_ = value; }
		}
		property Platform::String^ Host
		{
			Platform::String^ get() { return Host_; }
			void set( Platform::String^ value ) { Host_ = value; }
		}
		property int32 Port
		{
			int32 get() { return Port_; }
			void set( int32 value ) { Port_ = value; }
		}
		property Platform::String^ App
		{
			Platform::String^ get() { return App_; }
			void set( Platform::String^ value ) { App_ = value; }
		}
		property Platform::String^ Instance
		{
			Platform::String^ get() { return Instance_; }
			void set( Platform::String^ value ) { Instance_ = value; }
		}

	private:
		RtmpScheme Scheme_;
		Platform::String^ Host_;
		int32 Port_;
		Platform::String^ App_;
		Platform::String^ Instance_;
	};

} }