#pragma once
#include "VideoInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamVideoReceivedEventArgs sealed
	{
	internal:
		NetStreamVideoReceivedEventArgs();

		void SetDecodeTimestamp( int64 decodeTimestamp );
		void SetPresentationTimestamp( int64 presentationTimestamp );
		void SetData( std::vector<uint8> data, const size_t offset = 0 );

		Windows::Media::Core::MediaStreamSample^ CreateSample();

	public:
		property VideoInfo^ Info
		{
			VideoInfo^ get() { return Info_; }
		internal:
			void set( VideoInfo^ value ) { Info_ = value; }
		}
		property bool IsKeyframe
		{
			bool get() { return IsKeyframe_; }
		internal:
			void set( bool value ) { IsKeyframe_ = value; }
		}
		property Windows::Foundation::TimeSpan DecodeTimestamp
		{
			Windows::Foundation::TimeSpan get() { return DecodeTimestamp_; }
		}
		property Windows::Foundation::TimeSpan PresentationTimestamp
		{
			Windows::Foundation::TimeSpan get() { return PresentationTimestamp_; }
		}
		property Windows::Storage::Streams::IBuffer^ Data
		{
			Windows::Storage::Streams::IBuffer^ get() { return Data_; }
		}

	private:
		VideoInfo^ Info_;
		bool IsKeyframe_;
		Windows::Foundation::TimeSpan DecodeTimestamp_, PresentationTimestamp_;
		Windows::Storage::Streams::IBuffer^ Data_;
	};

} }