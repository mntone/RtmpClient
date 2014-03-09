#include "pch.h"
#include "MainPage.xaml.h"

using namespace Mntone::Rtmp::DemoApp;
using namespace Mntone::Rtmp::Client;
namespace WF = Windows::Foundation;
namespace WUIX = Windows::UI::Xaml;

MainPage::MainPage()
{
	InitializeComponent();
}

void MainPage::OnPageUnloaded( Platform::Object^ sender, WUIX::RoutedEventArgs^ e )
{
	CloseClient();
}

void MainPage::OnButtonClicked( Platform::Object^ sender, WUIX::RoutedEventArgs^ e )
{
	CloseClient();

	auto uri = Uri->Text;

	client_ = ref new SimpleVideoClient();
	client_->Started += ref new WF::EventHandler<SimpleVideoClientStartedEventArgs^>( this, &MainPage::OnStarted );
	client_->Stopped += ref new WF::EventHandler<SimpleVideoClientStoppedEventArgs^>( this, &MainPage::OnStopped );
	client_->ConnectAsync( ref new WF::Uri( uri ) );

	auto smtc = Windows::Media::SystemMediaTransportControls::GetForCurrentView();
	smtc->ButtonPressed += ref new WF::TypedEventHandler<Windows::Media::SystemMediaTransportControls^, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^>( this, &MainPage::OnMediaButtonPressed );
	smtc->IsEnabled = true;
	smtc->IsPlayEnabled = true;
	smtc->IsPauseEnabled = true;
	smtc->IsStopEnabled = true;
	smtc->PlaybackStatus = Windows::Media::MediaPlaybackStatus::Playing;
}

void MainPage::OnStarted( Platform::Object^ sender, SimpleVideoClientStartedEventArgs^ args )
{
	Dispatcher->RunAsync( Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler( [=]
	{
		mediaElement->SetMediaStreamSource( args->MediaStreamSource );
		mediaElement->MediaEnded += ref new WUIX::RoutedEventHandler( this, &MainPage::OnMediaEnded );
		mediaElement->Play();
	} ) );
}

void MainPage::OnStopped( Platform::Object^ sender, SimpleVideoClientStoppedEventArgs^ args )
{
}

void MainPage::OnMediaEnded( Platform::Object^ sender, WUIX::RoutedEventArgs^ e )
{
	foregroundElement->Visibility = WUIX::Visibility::Visible;
	CloseClient();
}

void MainPage::OnMediaButtonPressed( Windows::Media::SystemMediaTransportControls^ sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^ e )
{
	if( e->Button == Windows::Media::SystemMediaTransportControlsButton::Pause
		|| e->Button == Windows::Media::SystemMediaTransportControlsButton::Stop )
	{
		Dispatcher->RunAsync( Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler( [this]
		{
			mediaElement->Source = nullptr;

			auto smtc = Windows::Media::SystemMediaTransportControls::GetForCurrentView();
			smtc->PlaybackStatus = Windows::Media::MediaPlaybackStatus::Closed;
		} ) );
	}
}

void MainPage::CloseClient()
{
	if( client_ != nullptr )
	{
		delete client_;
		client_ = nullptr;
	}
}