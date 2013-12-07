#pragma once
#include "VideoInfo.h"

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
		property VideoInfo^ Info
		{
			VideoInfo^ get( void ) { return Info_; }
		internal:
			void set( VideoInfo^ value ) { Info_ = value; }
		}
		property bool IsKeyframe
		{
			bool get( void ) { return IsKeyframe_; }
		internal:
			void set( bool value ) { IsKeyframe_ = value; }
		}
		property Windows::Foundation::TimeSpan DecodeTimestamp
		{
			Windows::Foundation::TimeSpan get( void ) { return DecodeTimestamp_; }
		}
		property Windows::Foundation::TimeSpan PresentationTimestamp
		{
			Windows::Foundation::TimeSpan get( void ) { return PresentationTimestamp_; }
		}
		property Windows::Storage::Streams::IBuffer^ Data
		{
			Windows::Storage::Streams::IBuffer^ get( void ) { return Data_; }
		}

	private:
		VideoInfo^ Info_;
		bool IsKeyframe_;
		Windows::Foundation::TimeSpan DecodeTimestamp_, PresentationTimestamp_;
		Windows::Storage::Streams::IBuffer^ Data_;
	};

} }