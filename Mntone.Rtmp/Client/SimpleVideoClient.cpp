#include "pch.h"
#include "SimpleVideoClient.h"

using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace Mntone::Rtmp;
using namespace Mntone::Rtmp::Client;
namespace WF = Windows::Foundation;

SimpleVideoClient::SimpleVideoClient()
	: dispatcher_( Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher )
	, mediaStreamSource_( nullptr )
{ }

SimpleVideoClient::~SimpleVideoClient()
{
	CloseImpl();
}

void SimpleVideoClient::CloseImpl()
{
	if( connection_ != nullptr )
	{
		delete connection_;
		connection_ = nullptr;

		if( stream_ != nullptr )
		{
			delete stream_;
			stream_ = nullptr;
		}

		Stopped( this, ref new SimpleVideoClientStoppedEventArgs() );
	}
}

void SimpleVideoClient::CreateMediaStream( IMediaStreamDescriptor^ descriptor )
{
	mediaStreamSource_ = ref new MediaStreamSource( descriptor );

	WF::TimeSpan d;
	d.Duration = std::numeric_limits<uint64>::max();
	mediaStreamSource_->Duration = d;

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
	if( nsc == NetStatusCodeType::NetConnection_Connect_Success )
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
	else if( ( nsc & NetStatusCodeType::Level2Mask ) == NetStatusCodeType::NetConnection_Connect )
		CloseImpl();
}

void SimpleVideoClient::OnAttached( NetStream^ sender, NetStreamAttachedEventArgs^ args )
{
	stream_->Play( connection_->Uri->Instance );
}

void SimpleVideoClient::OnNetStreamStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args )
{
	const auto& nsc = args->NetStatusCode;
	if( nsc == NetStatusCodeType::NetStream_Play_Start )
	{
	}
}

void SimpleVideoClient::OnAudioStarted( NetStream^ sender, NetStreamAudioStartedEventArgs^ args )
{
	using namespace Windows::UI::Core;
	dispatcher_->RunAsync( CoreDispatcherPriority::Normal, ref new DispatchedHandler( [=]
	{
		const auto& info = args->Info;

		AudioEncodingProperties^ prop;
		if( info->Format == AudioFormat::Mp3 )
			prop = AudioEncodingProperties::CreateMp3( info->SampleRate, info->ChannelCount, info->Bitrate );
		else if( info->Format == AudioFormat::Aac )
			prop = AudioEncodingProperties::CreateAac( info->SampleRate, info->ChannelCount, info->Bitrate );
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

	using namespace Windows::UI::Core;
	dispatcher_->RunAsync( CoreDispatcherPriority::Normal, ref new DispatchedHandler( [=]
	{
		auto prop = VideoEncodingProperties::CreateH264();
		prop->ProfileId = H264ProfileIds::High;
		auto des = ref new VideoStreamDescriptor( prop );

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

void SimpleVideoClient::OnStarting( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs^ args )
{
	auto request = args->Request;
	WF::TimeSpan ts;
	ts.Duration = 0;
	request->SetActualStartPosition( ts );
}

void SimpleVideoClient::OnSampleRequested( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs^ args )
{
	const auto& request = args->Request;
	auto deferral = request->GetDeferral();

	if( request->StreamDescriptor->GetType()->FullName == Windows::Media::Core::AudioStreamDescriptor::typeid->FullName )
	{
		NetStreamAudioReceivedEventArgs^ data;
		{
			std::unique_lock<std::mutex> lock( audioMutex_ );
			while( audioBuffer_.empty() )
				audioConditionVariable_.wait( lock );
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
			while( videoBuffer_.empty() )
				videoConditionVariable_.wait( lock );
			data = videoBuffer_.front();
			videoBuffer_.pop();
		}
		request->Sample = data->CreateSample();
	}
	deferral->Complete();
}