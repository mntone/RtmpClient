#pragma once
#include "AudioInfo.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamAudioReceivedEventArgs sealed
	{
	internal:
		NetStreamAudioReceivedEventArgs( void );

		void SetTimestamp( int64 timestamp );
		void SetData( std::vector<uint8> data, const size_t offset = 0 );

		Windows::Media::Core::MediaStreamSample^ CreateSample( void );

	public:
		property AudioInfo^ Info
		{
			AudioInfo^ get( void ) { return Info_; }
		internal:
			void set( AudioInfo^ value ) { Info_ = value; }
		}
		property Windows::Foundation::TimeSpan Timestamp
		{
			Windows::Foundation::TimeSpan get( void ) { return Timestamp_; }
		}
		property Windows::Storage::Streams::IBuffer^ Data
		{
			Windows::Storage::Streams::IBuffer^ get( void ) { return Data_; }
		}

	private:
		AudioInfo^ Info_;
		Windows::Foundation::TimeSpan Timestamp_;
		Windows::Storage::Streams::IBuffer^ Data_;
	};

} }