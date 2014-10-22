#include "pch.h"
#include "SimpleVideoClient.h"

using namespace Concurrency;
using namespace Windows::Foundation;
using namespace Windows::Media::Core;
using namespace Mntone::Rtmp;
using namespace Mntone::Rtmp::Client;
namespace WMM = Windows::Media::MediaProperties;
namespace WUIC = Windows::UI::Core;

SimpleVideoClient::SimpleVideoClient()
	: connection_( nullptr )
	, stream_( nullptr )
	, mediaStreamSource_( nullptr )
	, bufferingHelper_( nullptr )
{ }

SimpleVideoClient::~SimpleVideoClient()
{
	CloseImpl();
}

void SimpleVideoClient::CloseImpl()
{
	if( mediaStreamSource_ != nullptr )
	{
		mediaStreamSource_->Starting -= startingEventToken_;
		mediaStreamSource_->SampleRequested -= sampleRequestedEventToken_;
		mediaStreamSource_ = nullptr;

		stream_->Attached -= streamAttachedEventToken_;
		stream_->StatusUpdated -= streamStatusUpdatedEventToken_;
		stream_->AudioStarted -= streamAudioStartedEventToken_;
		stream_->VideoStarted -= streamVideoStartedEventToken_;
		bufferingHelper_->Stop();
		bufferingHelper_ = nullptr;
		stream_ = nullptr;

		connection_->StatusUpdated -= connectionStatusUpdatedEventToken_;
		connection_ = nullptr;
	}
}

void SimpleVideoClient::CreateMediaStream( IMediaStreamDescriptor^ descriptor )
{
	CloseImpl();

	mediaStreamSource_ = ref new MediaStreamSource( descriptor );
	mediaStreamSource_->BufferTime = TimeSpan{ 5 * 10000000 };
	mediaStreamSource_->Duration = TimeSpan{ std::numeric_limits<int64>::max() };

	startingEventToken_ = mediaStreamSource_->Starting += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>( this, &SimpleVideoClient::OnStarting );
	sampleRequestedEventToken_ = mediaStreamSource_->SampleRequested += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>( this, &SimpleVideoClient::OnSampleRequested );
}

IAsyncAction^ SimpleVideoClient::ConnectAsync( Uri^ uri )
{
	return ConnectAsync( ref new RtmpUri( uri ) );
}

IAsyncAction^ SimpleVideoClient::ConnectAsync( RtmpUri^ uri )
{
	connection_ = ref new NetConnection();
	connectionStatusUpdatedEventToken_ = connection_->StatusUpdated += ref new EventHandler<NetStatusUpdatedEventArgs^>( this, &SimpleVideoClient::OnNetConnectionStatusUpdated );
	return connection_->ConnectAsync( uri );
}

void SimpleVideoClient::OnNetConnectionStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args )
{
	auto nsc = args->NetStatusCode;
	if( nsc == NetStatusCodeType::NetConnectionConnectSuccess )
	{
		stream_ = ref new NetStream();
		streamAttachedEventToken_ = stream_->Attached += ref new EventHandler<NetStreamAttachedEventArgs^>( this, &SimpleVideoClient::OnAttached );
		streamStatusUpdatedEventToken_ = stream_->StatusUpdated += ref new EventHandler<NetStatusUpdatedEventArgs^>( this, &SimpleVideoClient::OnNetStreamStatusUpdated );
		streamAudioStartedEventToken_ = stream_->AudioStarted += ref new EventHandler<NetStreamAudioStartedEventArgs^>( this, &SimpleVideoClient::OnAudioStarted );
		streamVideoStartedEventToken_ = stream_->VideoStarted += ref new EventHandler<NetStreamVideoStartedEventArgs^>( this, &SimpleVideoClient::OnVideoStarted );
		bufferingHelper_ = ref new BufferingHelper( stream_ );
		stream_->AttachAsync( connection_ );
	}
	else if( ( nsc & NetStatusCodeType::Level2Mask ) == NetStatusCodeType::NetConnectionConnect )
	{
		CloseImpl();
		Stopped( this, ref new SimpleVideoClientStoppedEventArgs() );
	}
}

void SimpleVideoClient::OnAttached( Platform::Object^ sender, NetStreamAttachedEventArgs^ args )
{
	stream_->PlayAsync( connection_->Uri->Instance );
}

void SimpleVideoClient::OnNetStreamStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args )
{
	const auto nsc = args->NetStatusCode;
	if( nsc == NetStatusCodeType::NetStreamPlayStop )
	{
		CloseImpl();
		Stopped( this, ref new SimpleVideoClientStoppedEventArgs() );
	}
}

void SimpleVideoClient::OnAudioStarted( Platform::Object^ sender, NetStreamAudioStartedEventArgs^ args )
{
	using namespace Windows::UI::Core;

	const auto info = args->Info;

	WMM::AudioEncodingProperties^ prop;
	if( info->Format == Media::AudioFormat::Mp3 )
	{
		prop = WMM::AudioEncodingProperties::CreateMp3( info->SampleRate, info->ChannelCount, info->Bitrate );
	}
	else if( info->Format == Media::AudioFormat::Aac )
	{
		prop = WMM::AudioEncodingProperties::CreateAac( info->SampleRate, info->ChannelCount, info->Bitrate );
	}
	else
	{
		if( mediaStreamSource_ != nullptr )
		{
			Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
		}
		return;
	}

	prop->BitsPerSample = info->BitsPerSample;

	const auto des = ref new AudioStreamDescriptor( prop );
	if( mediaStreamSource_ != nullptr )
	{
		mediaStreamSource_->AddStreamDescriptor( des );
		Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
	}
	else
	{
		CreateMediaStream( des );
		if( args->AudioOnly )
		{
			Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
		}
	}
}

void SimpleVideoClient::OnVideoStarted( Platform::Object^ sender, NetStreamVideoStartedEventArgs^ args )
{
	WMM::VideoEncodingProperties^ prop;
	if( args->Info->Format == Media::VideoFormat::Avc )
	{
		prop = WMM::VideoEncodingProperties::CreateH264();
		prop->ProfileId = static_cast<int32>( args->Info->ProfileIndication );
	}
	//else if( args->Info->Format == Media::VideoFormat::SorensonH263 )
	//{
	//	prop = ref new WMM::VideoEncodingProperties();
	//	prop->Subtype = WMM::MediaEncodingSubtypes::H263;
	//}
	else
	{
		if( mediaStreamSource_ != nullptr )
		{
			Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
		}
		return;
	}

	prop->Bitrate = args->Info->Bitrate;
	prop->Height = args->Info->Height;
	prop->Width = args->Info->Width;

	const auto des = ref new VideoStreamDescriptor( prop );
	if( mediaStreamSource_ != nullptr )
	{
		mediaStreamSource_->AddStreamDescriptor( des );
		Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
	}
	else
	{
		CreateMediaStream( des );
		if( args->VideoOnly )
		{
			Started( this, ref new SimpleVideoClientStartedEventArgs( mediaStreamSource_ ) );
		}
	}
}

void SimpleVideoClient::OnStarting( MediaStreamSource^ sender, MediaStreamSourceStartingEventArgs^ args )
{
	auto request = args->Request;
	TimeSpan ts;
	ts.Duration = 0;
	request->SetActualStartPosition( ts );
}

void SimpleVideoClient::OnSampleRequested( MediaStreamSource^ sender, MediaStreamSourceSampleRequestedEventArgs^ args )
{
	const auto request = args->Request;
	auto deferral = request->GetDeferral();

	if( request->StreamDescriptor->GetType()->FullName == AudioStreamDescriptor::typeid->FullName )
	{
		create_task( bufferingHelper_->GetAudioAsync() ).then( [=]( Windows::Media::Core::MediaStreamSample^ sample )
		{
			request->Sample = sample;
			deferral->Complete();
		} );
	}
	else
	{
		create_task( bufferingHelper_->GetVideoAsync() ).then( [=]( Windows::Media::Core::MediaStreamSample^ sample )
		{
			request->Sample = sample;
			deferral->Complete();
		} );
	}
}