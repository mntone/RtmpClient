#pragma once
#include "NetConnection.h"
#include "NetStream.h"
#include "SimpleVideoClientStartedEventArgs.h"
#include "SimpleVideoClientStoppedEventArgs.h"
#include "BufferingHelper.h"

namespace Mntone { namespace Rtmp { namespace Client {

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WINBLUE )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::Both )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SimpleVideoClient sealed
	{
	public:
		SimpleVideoClient();

		Windows::Foundation::IAsyncAction^ ConnectAsync( Windows::Foundation::Uri^ uri );
		[Windows::Foundation::Metadata::DefaultOverload] Windows::Foundation::IAsyncAction^ ConnectAsync( RtmpUri^ uri );

	private:
		~SimpleVideoClient();

		void CloseImpl();
		void CreateMediaStream( Windows::Media::Core::IMediaStreamDescriptor^ descriptor );

		// NetConnection
		void OnNetConnectionStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args );

		// NetStream
		void OnAttached( Platform::Object^ sender, NetStreamAttachedEventArgs^ args );
		void OnNetStreamStatusUpdated( Platform::Object^ sender, NetStatusUpdatedEventArgs^ args );
		void OnAudioStarted( Platform::Object^ sender, NetStreamAudioStartedEventArgs^ args );
		void OnVideoStarted( Platform::Object^ sender, NetStreamVideoStartedEventArgs^ args );

		// MediaStreamSource
		void OnStarting( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs^ args );
		void OnSampleRequested( Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs^ args );

	public:
		event Windows::Foundation::EventHandler<SimpleVideoClientStartedEventArgs^>^ Started;
		event Windows::Foundation::EventHandler<SimpleVideoClientStoppedEventArgs^>^ Stopped;

	private:
		Windows::Foundation::EventRegistrationToken
			startingEventToken_,
			sampleRequestedEventToken_,
			connectionStatusUpdatedEventToken_,
			streamAttachedEventToken_,
			streamStatusUpdatedEventToken_,
			streamAudioStartedEventToken_,
			streamVideoStartedEventToken_;

		NetConnection^ connection_;
		NetStream^ stream_;
		Windows::Media::Core::MediaStreamSource^ mediaStreamSource_;

		BufferingHelper^ bufferingHelper_;
	};

} } }