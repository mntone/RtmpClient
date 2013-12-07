#include "pch.h"
#include "NetStreamVideoReceivedEventArgs.h"

using namespace Mntone::Rtmp;

NetStreamVideoReceivedEventArgs::NetStreamVideoReceivedEventArgs()
{ }

void NetStreamVideoReceivedEventArgs::SetDecodeTimestamp( int64 decodeTimestamp )
{
	DecodeTimestamp_.Duration = decodeTimestamp * 10000ll;
}

void NetStreamVideoReceivedEventArgs::SetPresentationTimestamp( int64 presentationTimestamp )
{
	PresentationTimestamp_.Duration = presentationTimestamp * 10000ll;
}

void NetStreamVideoReceivedEventArgs::SetData( std::vector<uint8> data, const size_t offset )
{
	auto buf = ref new Windows::Storage::Streams::DataWriter();
	buf->WriteBytes( Platform::ArrayReference<uint8>( data.data() + offset, static_cast<uint32>( data.size() - offset ) ) );
	Data_ = buf->DetachBuffer();
}

Windows::Media::Core::MediaStreamSample^ NetStreamVideoReceivedEventArgs::CreateSample()
{
	const auto& sample = Windows::Media::Core::MediaStreamSample::CreateFromBuffer( Data_, PresentationTimestamp_ );
	sample->DecodeTimestamp = DecodeTimestamp_;
	sample->KeyFrame = IsKeyframe_;
	return std::move( sample );
}