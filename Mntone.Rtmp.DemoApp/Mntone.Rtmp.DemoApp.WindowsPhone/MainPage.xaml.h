#pragma once
#include "MainPage.g.h"

namespace Mntone { namespace Rtmp { namespace DemoApp {

	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void OnPageUnloaded( Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e );
		void OnButtonClicked( Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e );
		void OnStarted( Platform::Object^ sender, Mntone::Rtmp::Client::SimpleVideoClientStartedEventArgs^ args );
		void OnStopped( Platform::Object^ sender, Mntone::Rtmp::Client::SimpleVideoClientStoppedEventArgs^ args );

		void OnCurrentStateChanged( ::Platform::Object^ sender, ::Windows::UI::Xaml::RoutedEventArgs^ e );
		void OnMediaEnded( ::Platform::Object^ sender, ::Windows::UI::Xaml::RoutedEventArgs^ e );

		void OnMediaButtonPressed( Windows::Media::SystemMediaTransportControls^ sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^ e );

		void CloseClient();

	private:
		::Windows::Foundation::EventRegistrationToken startedEventToken_, stoppedEventToken_;

		App^ myAppContext_;
		Client::SimpleVideoClient^ client_;
	};

} } }
