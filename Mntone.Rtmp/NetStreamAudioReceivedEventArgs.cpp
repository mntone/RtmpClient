#include "pch.h"
#include "NetStreamAudioReceivedEventArgs.h"

namespace Mntone { namespace Rtmp {

	NetStreamAudioReceivedEventArgs::NetStreamAudioReceivedEventArgs( void )
	{ }

	void NetStreamAudioReceivedEventArgs::SetTimestamp( int64 timestamp )
	{
		_Timestamp.Duration = timestamp * 10000ll;
	}

	void NetStreamAudioReceivedEventArgs::SetData( std::vector<uint8> data, const size_t offset )
	{
		auto buf = ref new Windows::Storage::Streams::DataWriter();
		buf->WriteBytes( Platform::ArrayReference<uint8>( data.data() + offset, static_cast<uint32>( data.size() - offset ) ) );
		_Data = buf->DetachBuffer();
	}

} }