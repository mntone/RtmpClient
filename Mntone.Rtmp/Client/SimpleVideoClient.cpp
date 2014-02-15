#include "pch.h"
#include "SimpleVideoClient.h"

using namespace Windows::Media::Core;
using namespace Mntone::Rtmp;
using namespace Mntone::Rtmp::Client;
namespace WMM = Windows::Media::MediaProperties;
namespace WF = Windows::Foundation;
namespace WUIC = Windows::UI::Core;

SimpleVideoClient::SimpleVideoClient()
	: dispatcher_( WUIC::CoreWindow::GetForCurrentThread()->Dispatcher )
	, mediaStreamSource_( nullptr )
	, isEnable_( true )
{ }

SimpleVideoClient::~SimpleVideoClient()
{
	CloseImpl();
}

void SimpleVideoClient::CloseImpl()
{
	if( connection_ != nullptr )
	{
		if( stream_ != nullptr )
		{
			delete stream_;
			stream_ = nullptr;
		}

		delete connection_;
		connection_ = nullptr;

		Stopped( this, ref new SimpleVideoClientStoppedEventArgs() );
	}
}

void SimpleVideoClient::CreateMediaStream( IMediaStreamDescriptor^ descriptor )
{
	mediaStreamSource_ = ref new MediaStreamSource( descriptor );

	WF::TimeSpan duration;
	duration.Duration = std::numeric_limits<uint64>::max();
	mediaStreamSource_->Duration = duration;

	mediaStreamSource_->Starting += ref new WF::TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>( this, &SimpleVideoClient::OnStarting );
	mediaStreamSource_->SampleRequested += ref new WF::TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>( this, &SimpleVideoClient::OnSampleRequested );
}

void SimpleVideoClient::Connect( WF::Uri^ uri )
{
	Connect( ref new RtmpUri( uri ) );
}

void SimpleVideoClient::Connect( RtmpUri^ uri )
{
	connection_ = ref new NetConnection();
	connection_->StatusUpdated += ref new WF::EventHandler<NetStatusUpdatedEventArgs^>( this, &SimpleVideoClient::OnNetConnectionStatusUpdated );
	connection_->ConnectAsync( uri );
}

void SimpleVideoClient::OnNetConnectionStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args )
{
	auto nsc = args->NetStatusCode;
	if( nsc == NetStatusCodeType::NetConnectionConnectSuccess )
	{
		stream_ = ref new NetStream();
		stream_->Attached += ref new WF::TypedEventHandler<NetStream^, NetStreamAttachedEventArgs^>( this, &SimpleVideoClient::OnAttached );
		stream_->StatusUpdated += ref new WF::EventHandler<NetStatusUpdatedEventArgs^>( this, &SimpleVideoClient::OnNetStreamStatusUpdated );
		stream_->AudioStarted += ref new WF::TypedEventHandler<NetStream^, NetStreamAudioStartedEventArgs^>( this, &SimpleVideoClient::OnAudioStarted );
		stream_->AudioReceived += ref new WF::TypedEventHandler<NetStream^, NetStreamAudioReceivedEventArgs^>( this, &SimpleVideoClient::OnAudioReceived );
		stream_->VideoStarted += ref new WF::TypedEventHandler<NetStream^, NetStreamVideoStartedEventArgs^>( this, &SimpleVideoClient::OnVideoStarted );
		stream_->VideoReceived += ref new WF::TypedEventHandler<NetStream^, NetStreamVideoReceivedEventArgs^>( this, &SimpleVideoClient::OnVideoReceived );
		stream_->AttachAsync( connection_ );
	}
	else if( ( nsc & NetStatusCodeType::Level2Mask ) == NetStatusCodeType::NetConnectionConnect )
		CloseImpl();
}

void SimpleVideoClient::OnAttached( NetStream^ sender, NetStreamAttachedEventArgs^ args )
{
	stream_->PlayAsync( connection_->Uri->Instance );
}

void SimpleVideoClient::OnNetStreamStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args )
{
	const auto nsc = args->NetStatusCode;
	if( nsc == NetStatusCodeType::NetStreamPlayStop )
	{
		{
			std::unique_lock<std::mutex> lock_audio( audioMutex_, std::defer_lock );
			std::unique_lock<std::mutex> lock_video( videoMutex_, std::defer_lock );
			std::lock( lock_audio, lock_video );
			isEnable_ = false;
			audioConditionVariable_.notify_all();
			videoConditionVariable_.notify_all();
		}

		Stopped( this, ref new SimpleVideoClientStoppedEventArgs() );
	}
}

void SimpleVideoClient::OnAudioStarted( NetStream^ sender, NetStreamAudioStartedEventArgs^ args )
{
	using namespace Windows::UI::Core;
	dispatcher_->RunAsync( CoreDispatcherPriority::Normal, ref new DispatchedHandler( [=]
	{
		const auto info = args->Info;

		WMM::AudioEncodingProperties^ prop;
		if( info->Format == AudioFormat::Mp3 )
			prop = WMM::AudioEncodingProperties::CreateMp3( info->SampleRate, info->ChannelCount, info->Bitrate );
		else if( info->Format == AudioFormat::Aac )
			prop = WMM::AudioEncodingProperties::CreateAac( info->SampleRate, info->ChannelCount, info->Bitrate );
		else
			return;

		prop->BitsPerSample = info->BitsPerSample;
		const auto des = ref new AudioStreamDescriptor( prop );

		if( mediaStreamSource_ != nullptr )
		{
			mediaStreamSource_->AddStreamDescriptor( des );	
			Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
		}
		else
			CreateMediaStream( des );
	} ) );
}

void SimpleVideoClient::OnAudioReceived( NetStream^ sender, NetStreamAudioReceivedEventArgs^ args )
{
	std::lock_guard<std::mutex> lock( audioMutex_ );
	audioBuffer_.emplace( args );
	audioConditionVariable_.notify_one();
}

void SimpleVideoClient::OnVideoStarted( NetStream^ sender, NetStreamVideoStartedEventArgs^ args )
{
	if( args->Info->Format != VideoFormat::Avc )
		return;

	dispatcher_->RunAsync( WUIC::CoreDispatcherPriority::Normal, ref new WUIC::DispatchedHandler( [=]
	{
		auto prop = WMM::VideoEncodingProperties::CreateH264();
		prop->ProfileId = WMM::H264ProfileIds::High;
		const auto des = ref new VideoStreamDescriptor( prop );

		if( mediaStreamSource_ != nullptr )
		{
			mediaStreamSource_->AddStreamDescriptor( des );
			Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
		}
		else
			CreateMediaStream( des );
	} ) );
}

void SimpleVideoClient::OnVideoReceived( NetStream^ sender, NetStreamVideoReceivedEventArgs^ args )
{
	std::lock_guard<std::mutex> lock( videoMutex_ );
	videoBuffer_.emplace( args );
	videoConditionVariable_.notify_one();
}

void SimpleVideoClient::OnStarting( MediaStreamSource^ sender, MediaStreamSourceStartingEventArgs^ args )
{
	auto request = args->Request;
	WF::TimeSpan ts;
	ts.Duration = 0;
	request->SetActualStartPosition( ts );
}

void SimpleVideoClient::OnSampleRequested( MediaStreamSource^ sender, MediaStreamSourceSampleRequestedEventArgs^ args )
{
	const auto request = args->Request;
	auto deferral = request->GetDeferral();

	if( request->StreamDescriptor->GetType()->FullName == AudioStreamDescriptor::typeid->FullName )
	{
		NetStreamAudioReceivedEventArgs^ data;
		{
			std::unique_lock<std::mutex> lock( audioMutex_ );
			audioConditionVariable_.wait( lock, [&] { return !isEnable_ || !audioBuffer_.empty(); } );
			if( !isEnable_ )
			{
				deferral->Complete();
				return;
			}

			data = audioBuffer_.front();
			audioBuffer_.pop();
		}
		request->Sample = data->CreateSample();
	}
	else
	{
		NetStreamVideoReceivedEventArgs^ data;
		{
			std::unique_lock<std::mutex> lock( videoMutex_ );
			videoConditionVariable_.wait( lock, [&] { return !isEnable_ || !videoBuffer_.empty(); } );
			if( !isEnable_ )
			{
				deferral->Complete();
				return;
			}

			data = videoBuffer_.front();
			videoBuffer_.pop();
		}
		request->Sample = data->CreateSample();
	}
	deferral->Complete();
}