#include "pch.h"
#include "SimpleVideoClient.h"

using namespace Mntone::Rtmp;
using namespace Mntone::Rtmp::Client;
namespace WF = Windows::Foundation;

SimpleVideoClient::SimpleVideoClient( void ) :
	_dispatcher( Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher ),
	_mediaStreamSource( nullptr )
{ }

SimpleVideoClient::~SimpleVideoClient( void )
{
	__Close();
}

void SimpleVideoClient::__Close( void )
{
	if( _connection != nullptr )
	{
		delete _connection;
		_connection = nullptr;

		if( _stream != nullptr )
		{
			delete _stream;
			_stream = nullptr;
		}

		Stopped( this, ref new SimpleVideoClientStoppedEventArgs() );
	}
}

void SimpleVideoClient::Connect( WF::Uri^ uri )
{
	Connect( ref new RtmpUri( uri ) );
}

void SimpleVideoClient::Connect( RtmpUri^ uri )
{
	_connection = ref new NetConnection();
	_connection->StatusUpdated += ref new WF::EventHandler<NetStatusUpdatedEventArgs^>( this, &SimpleVideoClient::OnNetConnectionStatusUpdated );
	_connection->Connect( uri );
}

void SimpleVideoClient::OnNetConnectionStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args )
{
	auto ns = args->NetStatus;
	if( ns == NetStatusType::NetConnection_Connect_Success )
	{
		_stream = ref new NetStream();
		_stream->Attached += ref new WF::TypedEventHandler<NetStream^, NetStreamAttachedEventArgs^>( this, &SimpleVideoClient::OnAttached );
		_stream->StatusUpdated += ref new WF::EventHandler<NetStatusUpdatedEventArgs^>( this, &SimpleVideoClient::OnNetStreamStatusUpdated );
		_stream->AudioStarted += ref new WF::TypedEventHandler<NetStream^, NetStreamAudioStartedEventArgs^>( this, &SimpleVideoClient::OnAudioStarted );
		_stream->AudioReceived += ref new WF::TypedEventHandler<NetStream^, NetStreamAudioReceivedEventArgs^>( this, &SimpleVideoClient::OnAudioReceived );
		_stream->VideoStarted += ref new WF::TypedEventHandler<NetStream^, NetStreamVideoStartedEventArgs^>( this, &SimpleVideoClient::OnVideoStarted );
		_stream->VideoReceived += ref new WF::TypedEventHandler<NetStream^, NetStreamVideoReceivedEventArgs^>( this, &SimpleVideoClient::OnVideoReceived );
		_stream->Attach( _connection );
	}
	else if( ( ns & NetStatusType::NetConnection_Connect ) == NetStatusType::NetConnection_Connect )
		__Close();
}

void SimpleVideoClient::OnAttached( NetStream^ sender, NetStreamAttachedEventArgs^ args )
{
	_stream->Play( _connection->Uri->Instance );
}

void SimpleVideoClient::OnNetStreamStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args )
{
	auto ns = args->NetStatus;
	if( ns == NetStatusType::NetStream_Play_Start )
	{
	}
}

void SimpleVideoClient::OnAudioStarted( NetStream^ sender, NetStreamAudioStartedEventArgs^ args )
{
	using namespace Windows::UI::Core;
	_dispatcher->RunAsync( CoreDispatcherPriority::Normal, ref new DispatchedHandler( [=]
	{
		using namespace Windows::Media::Core;
		using namespace Windows::Media::MediaProperties;

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

		if( _mediaStreamSource == nullptr )
		{
			_mediaStreamSource = ref new MediaStreamSource( des );

			WF::TimeSpan d;
			d.Duration = INT64_MAX;
			_mediaStreamSource->Duration = d;

			_mediaStreamSource->Starting += ref new WF::TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>( this, &SimpleVideoClient::OnStarting );
			_mediaStreamSource->SampleRequested += ref new WF::TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>( this, &SimpleVideoClient::OnSampleRequested );
		}
		else
		{
			_mediaStreamSource->AddStreamDescriptor( des );	
			Started( this, ref new SimpleVideoClientStartedEventArgs( _mediaStreamSource ) );
		}
	} ) );
}

void SimpleVideoClient::OnAudioReceived( NetStream^ sender, NetStreamAudioReceivedEventArgs^ args )
{
	std::lock_guard<std::mutex> lock( _audioMutex );
	_audioBuffer.emplace( args );
	_audioConditionVariable.notify_one();
}

void SimpleVideoClient::OnVideoStarted( NetStream^ sender, NetStreamVideoStartedEventArgs^ args )
{
	if( args->Info->Format != VideoFormat::Avc )
		return;

	using namespace Windows::UI::Core;
	_dispatcher->RunAsync( CoreDispatcherPriority::Normal, ref new DispatchedHandler( [=]
	{
		using namespace Windows::Media::Core;
		using namespace Windows::Media::MediaProperties;

		auto prop = VideoEncodingProperties::CreateH264();
		prop->ProfileId = H264ProfileIds::High;
		auto des = ref new VideoStreamDescriptor( prop );

		if( _mediaStreamSource == nullptr )
		{
			_mediaStreamSource = ref new MediaStreamSource( des );

			WF::TimeSpan d;
			d.Duration = INT64_MAX;
			_mediaStreamSource->Duration = d;

			_mediaStreamSource->Starting += ref new WF::TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>( this, &SimpleVideoClient::OnStarting );
			_mediaStreamSource->SampleRequested += ref new WF::TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>( this, &SimpleVideoClient::OnSampleRequested );
		}
		else
		{
			_mediaStreamSource->AddStreamDescriptor( des );		
			Started( this, ref new SimpleVideoClientStartedEventArgs( _mediaStreamSource ) );
		}
	} ) );
}

void SimpleVideoClient::OnVideoReceived( NetStream^ sender, NetStreamVideoReceivedEventArgs^ args )
{
	std::lock_guard<std::mutex> lock( _videoMutex );
	_videoBuffer.emplace( args );
	_videoConditionVariable.notify_one();
}

void SimpleVideoClient::OnStarting( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs^ args )
{
	auto request = args->Request;
	Windows::Foundation::TimeSpan ts;
	ts.Duration = 0;
	request->SetActualStartPosition( ts );
}

void SimpleVideoClient::OnSampleRequested( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs^ args )
{
	auto request = args->Request;
	auto deferral = request->GetDeferral();

	if( request->StreamDescriptor->GetType()->FullName == Windows::Media::Core::AudioStreamDescriptor::typeid->FullName )
	{
		NetStreamAudioReceivedEventArgs^ data;
		{
			std::unique_lock<std::mutex> lock( _audioMutex );
			while( _audioBuffer.empty() )
				_audioConditionVariable.wait( lock );
			data = _audioBuffer.front();
			_audioBuffer.pop();
		}
		request->Sample = data->CreateSample();
	}
	else
	{
		NetStreamVideoReceivedEventArgs^ data;
		{
			std::unique_lock<std::mutex> lock( _videoMutex );
			while( _videoBuffer.empty() )
				_videoConditionVariable.wait( lock );
			data = _videoBuffer.front();
			_videoBuffer.pop();
		}
		request->Sample = data->CreateSample();
	}
	deferral->Complete();
}