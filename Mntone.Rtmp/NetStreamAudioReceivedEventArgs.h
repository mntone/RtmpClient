#pragma once

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStreamAudioReceivedEventArgs sealed
	{
	internal:
		NetStreamAudioReceivedEventArgs( void );

		void SetTimestamp( uint64 timestamp );
		void SetData( std::vector<uint8> data, const size_t offset = 0 );

	public:
		property Windows::Foundation::TimeSpan Timestamp
		{
			Windows::Foundation::TimeSpan get() { return _Timestamp; }
		}
		property Windows::Storage::Streams::IBuffer^ Data
		{
			Windows::Storage::Streams::IBuffer^ get() { return _Data; }
		}

	private:
		Windows::Foundation::TimeSpan _Timestamp;
		Windows::Storage::Streams::IBuffer^ _Data;
	};

} }