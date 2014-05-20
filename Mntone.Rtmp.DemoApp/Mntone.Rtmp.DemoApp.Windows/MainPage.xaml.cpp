#include "pch.h"
#include "MainPage.xaml.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Mntone::Rtmp::Client;
using namespace Mntone::Rtmp::DemoApp;

MainPage::MainPage()
{
	InitializeComponent();
}

void MainPage::OnPageUnloaded( Object^ sender, RoutedEventArgs^ e )
{
	CloseClient();
}

void MainPage::OnSizeChanged( Object^ sender, SizeChangedEventArgs^ e )
{
	if( e->NewSize.Width < 500 )
	{
		VisualStateManager::GoToState( this, "Compact", false );
	}
	else
	{
		VisualStateManager::GoToState( this, "FullSize", false );
	}
}

void MainPage::OnButtonClicked( Object^ sender, RoutedEventArgs^ e )
{
	CloseClient();

	auto uri = uriTextBox->Text;

	client_ = ref new SimpleVideoClient();
	startedEventToken_ = client_->Started += ref new EventHandler<SimpleVideoClientStartedEventArgs^>( this, &MainPage::OnStarted );
	stoppedEventToken_ = client_->Stopped += ref new EventHandler<SimpleVideoClientStoppedEventArgs^>( this, &MainPage::OnStopped );
	client_->ConnectAsync( ref new Uri( uri ) );

	auto smtc = Windows::Media::SystemMediaTransportControls::GetForCurrentView();
	smtc->ButtonPressed += ref new TypedEventHandler<Windows::Media::SystemMediaTransportControls^, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^>( this, &MainPage::OnMediaButtonPressed );
	smtc->IsEnabled = true;
	smtc->IsPlayEnabled = true;
	smtc->IsPauseEnabled = true;
	smtc->IsStopEnabled = true;
	smtc->PlaybackStatus = Windows::Media::MediaPlaybackStatus::Playing;
}

void MainPage::OnStarted( Object^ sender, SimpleVideoClientStartedEventArgs^ args )
{
	Dispatcher->RunAsync( CoreDispatcherPriority::High, ref new DispatchedHandler( [=]
	{
		mediaElement->SetMediaStreamSource( args->MediaStreamSource );
		mediaElement->MediaEnded += ref new RoutedEventHandler( this, &MainPage::OnMediaEnded );
		mediaElement->Play();
		foregroundElement->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	} ) );
}

void MainPage::OnStopped( Object^ sender, SimpleVideoClientStoppedEventArgs^ args )
{
}

void MainPage::OnMediaEnded( Object^ sender, RoutedEventArgs^ e )
{
	foregroundElement->Visibility = Windows::UI::Xaml::Visibility::Visible;
	CloseClient();
}

void MainPage::OnMediaButtonPressed( Windows::Media::SystemMediaTransportControls^ sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^ e )
{
	if( e->Button == Windows::Media::SystemMediaTransportControlsButton::Pause
		|| e->Button == Windows::Media::SystemMediaTransportControlsButton::Stop )
	{
		Dispatcher->RunAsync( CoreDispatcherPriority::Low, ref new DispatchedHandler( [this]
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
		client_->Started -= startedEventToken_;
		client_->Stopped -= stoppedEventToken_;
		client_ = nullptr;
	}
}