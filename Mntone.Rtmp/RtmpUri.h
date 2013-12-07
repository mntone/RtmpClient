#pragma once
#include "RtmpScheme.h"

namespace Mntone { namespace Rtmp {

	public ref class RtmpUri sealed
	{
	public:
		RtmpUri( Platform::String^ uri );
		[Windows::Foundation::Metadata::DefaultOverload]
		RtmpUri( Windows::Foundation::Uri^ uri );

		// IStringable
		virtual Platform::String^ ToString( void ) override sealed;

	private:
		void ParseScheme( Platform::String^ schemeName );
		void ParsePort( int32 port );
		void ParsePath( Platform::String^ path );

	public:
		property RtmpScheme Scheme
		{
			RtmpScheme get( void ) { return Scheme_; }
		}
		property Platform::String^ Host
		{
			Platform::String^ get( void ) { return Host_; }
		}
		property int32 Port
		{
			int32 get( void ) { return Port_; }
		}
		property Platform::String^ App
		{
			Platform::String^ get( void ) { return App_; }
		}
		property Platform::String^ Instance
		{
			Platform::String^ get( void ) { return Instance_; }
		}

	private:
		RtmpScheme Scheme_;
		Platform::String^ Host_;
		int32 Port_;
		Platform::String^ App_;
		Platform::String^ Instance_;
	};

} }