#include "pch.h"
#include "NetStreamAudioReceivedEventArgs.h"

using namespace Mntone::Rtmp;

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

Windows::Media::Core::MediaStreamSample^ NetStreamAudioReceivedEventArgs::CreateSample( void )
{
	const auto& sample = Windows::Media::Core::MediaStreamSample::CreateFromBuffer( _Data, _Timestamp );
	sample->KeyFrame = true;
	return std::move( sample );
}