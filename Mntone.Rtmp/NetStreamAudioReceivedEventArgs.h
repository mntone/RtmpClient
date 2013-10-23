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
			AudioInfo^ get( void ) { return _Info; }
		internal:
			void set( AudioInfo^ value ) { _Info = value; }
		}
		property Windows::Foundation::TimeSpan Timestamp
		{
			Windows::Foundation::TimeSpan get( void ) { return _Timestamp; }
		}
		property Windows::Storage::Streams::IBuffer^ Data
		{
			Windows::Storage::Streams::IBuffer^ get( void ) { return _Data; }
		}

	private:
		AudioInfo^ _Info;
		Windows::Foundation::TimeSpan _Timestamp;
		Windows::Storage::Streams::IBuffer^ _Data;
	};

} }