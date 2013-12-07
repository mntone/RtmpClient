#include "pch.h"
#include "NetStreamAudioReceivedEventArgs.h"

using namespace Mntone::Rtmp;

NetStreamAudioReceivedEventArgs::NetStreamAudioReceivedEventArgs()
{ }

void NetStreamAudioReceivedEventArgs::SetTimestamp( int64 timestamp )
{
	Timestamp_.Duration = timestamp * 10000ll;
}

void NetStreamAudioReceivedEventArgs::SetData( std::vector<uint8> data, const size_t offset )
{
	auto buf = ref new Windows::Storage::Streams::DataWriter();
	buf->WriteBytes( Platform::ArrayReference<uint8>( data.data() + offset, static_cast<uint32>( data.size() - offset ) ) );
	Data_ = buf->DetachBuffer();
}

Windows::Media::Core::MediaStreamSample^ NetStreamAudioReceivedEventArgs::CreateSample()
{
	const auto& sample = Windows::Media::Core::MediaStreamSample::CreateFromBuffer( Data_, Timestamp_ );
	sample->KeyFrame = true;
	return std::move( sample );
}