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
			RtmpScheme get( void ) { return _Scheme; }
		}
		property Platform::String^ Host
		{
			Platform::String^ get( void ) { return _Host; }
		}
		property int32 Port
		{
			int32 get( void ) { return _Port; }
		}
		property Platform::String^ App
		{
			Platform::String^ get( void ) { return _App; }
		}
		property Platform::String^ Instance
		{
			Platform::String^ get( void ) { return _Instance; }
		}

	private:
		RtmpScheme _Scheme;
		Platform::String^ _Host;
		int32 _Port;
		Platform::String^ _App;
		Platform::String^ _Instance;
	};

} }