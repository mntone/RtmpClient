#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "NetConnection.h"
#include "NetStream.h"
#include "SimpleVideoClientStartedEventArgs.h"
#include "SimpleVideoClientStoppedEventArgs.h"

namespace Mntone { namespace Rtmp { namespace Client {

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WINBLUE )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::STA )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SimpleVideoClient sealed
	{
	public:
		SimpleVideoClient( void );
		virtual ~SimpleVideoClient( void );

		void Connect( Windows::Foundation::Uri^ uri );
		[Windows::Foundation::Metadata::DefaultOverload] void Connect( RtmpUri^ uri );

	private:
		void __Close( void );

		// NetConnection
		void OnNetConnectionStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args );

		// NetStream
		void OnAttached( NetStream^ sender, NetStreamAttachedEventArgs^ args );
		void OnNetStreamStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args );
		void OnAudioStarted( NetStream^ sender, NetStreamAudioStartedEventArgs^ args );
		void OnAudioReceived( NetStream^ sender, NetStreamAudioReceivedEventArgs^ args );
		void OnVideoStarted( NetStream^ sender, NetStreamVideoStartedEventArgs^ args );
		void OnVideoReceived( NetStream^ sender, NetStreamVideoReceivedEventArgs^ args );

		// MediaStreamSource
		void OnStarting( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs^ args );
		void OnSampleRequested( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs^ args );

	public:
		event Windows::Foundation::EventHandler<SimpleVideoClientStartedEventArgs^>^ Started;
		event Windows::Foundation::TypedEventHandler<SimpleVideoClient^, SimpleVideoClientStoppedEventArgs^>^ Stopped;

	private:
		Windows::UI::Core::CoreDispatcher^ _dispatcher;

		NetConnection^ _connection;
		NetStream^ _stream;
		Windows::Media::Core::MediaStreamSource^ _mediaStreamSource;

		// Buffer
		mutable std::mutex _audioMutex, _videoMutex;
		std::condition_variable _audioConditionVariable, _videoConditionVariable;
		std::queue<NetStreamAudioReceivedEventArgs^> _audioBuffer;
		std::queue<NetStreamVideoReceivedEventArgs^> _videoBuffer;
	};

} } }