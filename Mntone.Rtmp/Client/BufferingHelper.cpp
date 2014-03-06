#include "pch.h"
#include "BufferingHelper.h"
#include "NetStream.h"
#include "NetStreamAudioReceivedEventArgs.h"
#include "NetStreamVideoReceivedEventArgs.h"

using namespace Concurrency;
using namespace Windows::Foundation;
using namespace Windows::Media::Core;
using namespace Mntone::Rtmp;
using namespace Mntone::Rtmp::Client;

BufferingHelper::BufferingHelper( NetStream^ stream )
	: isEnable_( true )
{
	stream->AudioReceived += ref new EventHandler<NetStreamAudioReceivedEventArgs^>( this, &BufferingHelper::OnAudioReceived );
	stream->VideoReceived += ref new EventHandler<NetStreamVideoReceivedEventArgs^>( this, &BufferingHelper::OnVideoReceived );
}

void BufferingHelper::Stop()
{
	std::unique_lock<std::mutex> lock_audio( audioMutex_, std::defer_lock );
	std::unique_lock<std::mutex> lock_video( videoMutex_, std::defer_lock );
	std::lock( lock_audio, lock_video );
	isEnable_ = false;
	audioConditionVariable_.notify_all();
	videoConditionVariable_.notify_all();
}

IAsyncOperation<MediaStreamSample^>^ BufferingHelper::GetAudioAsync()
{
	return create_async( [=]() -> MediaStreamSample^
	{
		std::unique_lock<std::mutex> lock( audioMutex_ );
		audioConditionVariable_.wait( lock, [&] { return !isEnable_ || !audioBuffer_.empty(); } );
		if( !isEnable_ )
		{
			return nullptr;
		}

		auto data = audioBuffer_.front();
		audioBuffer_.pop();
		return data;
	} );
}

IAsyncOperation<MediaStreamSample^>^ BufferingHelper::GetVideoAsync()
{
	return create_async( [=]() -> MediaStreamSample^
	{
		std::unique_lock<std::mutex> lock( videoMutex_ );
		videoConditionVariable_.wait( lock, [&] { return !isEnable_ || !videoBuffer_.empty(); } );
		if( !isEnable_ )
		{
			return nullptr;
		}

		auto data = videoBuffer_.front();
		videoBuffer_.pop();
		return data;
	} );
}

void BufferingHelper::OnAudioReceived( Platform::Object^ sender, NetStreamAudioReceivedEventArgs^ args )
{
	std::lock_guard<std::mutex> lock( audioMutex_ );
	audioBuffer_.emplace( args->CreateSample() );
	audioConditionVariable_.notify_one();
}

void BufferingHelper::OnVideoReceived( Platform::Object^ sender, NetStreamVideoReceivedEventArgs^ args )
{
	std::lock_guard<std::mutex> lock( videoMutex_ );
	videoBuffer_.emplace( args->CreateSample() );
	videoConditionVariable_.notify_one();
}