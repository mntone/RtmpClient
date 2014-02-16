#pragma once
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
		SimpleVideoClient();

		Windows::Foundation::IAsyncAction^ ConnectAsync( Windows::Foundation::Uri^ uri );
		[Windows::Foundation::Metadata::DefaultOverload] Windows::Foundation::IAsyncAction^ ConnectAsync( RtmpUri^ uri );

	private:
		void CloseImpl();
		void CreateMediaStream( Windows::Media::Core::IMediaStreamDescriptor^ descriptor );

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
		event Windows::Foundation::EventHandler<SimpleVideoClientStoppedEventArgs^>^ Stopped;

	private:
		Windows::UI::Core::CoreDispatcher^ dispatcher_;

		bool isEnable_;
		NetConnection^ connection_;
		NetStream^ stream_;
		Windows::Media::Core::MediaStreamSource^ mediaStreamSource_;

		// Buffer
		mutable std::mutex audioMutex_, videoMutex_;
		std::condition_variable audioConditionVariable_, videoConditionVariable_;
		std::queue<NetStreamAudioReceivedEventArgs^> audioBuffer_;
		std::queue<NetStreamVideoReceivedEventArgs^> videoBuffer_;
	};

} } }