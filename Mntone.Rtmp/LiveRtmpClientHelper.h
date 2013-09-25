#pragma once
#include "pch.h"

namespace Mntone { namespace Rtmp {


	//public ref class LiveRtmpClientHelper sealed
	//{
	//public:
	//	LiveRtmpClientHelper( void )
	//	{}

	//	void Initialize( Windows::Foundation::Uri^ uri );

	//	Windows::Media::MediaProperties::AudioEncodingProperties^ GetAudioProperties( void )
	//	{
	//		using namespace Windows::Media::MediaProperties;
	//		auto audio = AudioEncodingProperties::CreateMp3( 44100, 2, 112 );
	//		//auto audio = AudioEncodingProperties::CreateAac( 44100, 2, 112 );
	//		audio->BitsPerSample = 16;
	//		return audio;
	//	}

	//	Windows::Media::MediaProperties::VideoEncodingProperties^ GetVideoProperties( void )
	//	{
	//		using namespace Windows::Media::MediaProperties;
	//		auto video = VideoEncodingProperties::CreateH264();
	//		video->ProfileId = H264ProfileIds::High;
	//		video->Bitrate = 1024;
	//		video->FrameRate->Numerator = 20000;
	//		video->FrameRate->Denominator = 1000;
	//		video->Height = 720;
	//		video->Width = 1280;
	//		return video;
	//	}

	//	AudioData^ RequestAudio( void )
	//	{
	//		auto audioData = _client->GetAudioData();
	//		const auto& vec = std::get<2>( audioData );
	//		if( vec.size() == 0 )
	//			return nullptr;

	//		auto data = ref new AudioData( std::get<0>( audioData ), std::get<1>( audioData ), vec.size() );
	//		memcpy( data->Data->Data, vec.data(), vec.size() );
	//		return data;
	//	}

	//	VideoData^ RequestVideo( void )
	//	{
	//		auto videoData = _client->GetVideoData();
	//		const auto& vec = std::get<4>( videoData );
	//		if( vec.size() == 0 )
	//			return nullptr;

	//		auto data = ref new VideoData( std::get<0>( videoData ), std::get<1>( videoData ), std::get<2>( videoData ), std::get<3>( videoData ), vec.size() );
	//		memcpy( data->Data->Data, vec.data(), vec.size() );
	//		return data;
	//	}

	//private:
	//	void OnBufferCompleted( RtmpLiveClient^ client )
	//	{
	//		RaiseStarted();
	//	}

	//	void RaiseStarted( void )
	//	{
	//		//create_task( [this]
	//		//{
	//			Started( this );//, ref new LiveRtmpClientHelperLiveStartedEventArgs() );
	//		//}, _context );
	//	}

	//public:
	//	event RtmpDelegate^ Started;
	//	//event Windows::Foundation::TypedEventHandler<LiveRtmpClientHelper^, LiveRtmpClientHelperLiveStartedEventArgs^>^ Started;

	//private:
	//	RtmpLiveClient^ _client;
	//};

} }
