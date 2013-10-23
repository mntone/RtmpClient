#pragma once
#include "VideoFormat.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamVideoReceivedEventArgs sealed
	{
	internal:
		NetStreamVideoReceivedEventArgs( void );

		void SetDecodeTimestamp( int64 decodeTimestamp );
		void SetPresentationTimestamp( int64 presentationTimestamp );
		void SetData( std::vector<uint8> data, const size_t offset = 0 );

		Windows::Media::Core::MediaStreamSample^ CreateSample( void );

	public:
		property bool IsKeyframe
		{
			bool get( void ) { return _IsKeyframe; }
		internal:
			void set( bool value ) { _IsKeyframe = value; }
		}
		property Windows::Foundation::TimeSpan DecodeTimestamp
		{
			Windows::Foundation::TimeSpan get( void ) { return _DecodeTimestamp; }
		}
		property Windows::Foundation::TimeSpan PresentationTimestamp
		{
			Windows::Foundation::TimeSpan get( void ) { return _PresentationTimestamp; }
		}
		property VideoFormat Format
		{
			VideoFormat get( void ) { return _Format; }
		internal:
			void set( VideoFormat value ) { _Format = value; }
		}
		property Windows::Storage::Streams::IBuffer^ Data
		{
			Windows::Storage::Streams::IBuffer^ get( void ) { return _Data; }
		}

	private:
		bool _IsKeyframe;
		Windows::Foundation::TimeSpan _DecodeTimestamp, _PresentationTimestamp;
		VideoFormat _Format;
		Windows::Storage::Streams::IBuffer^ _Data;
	};

} }