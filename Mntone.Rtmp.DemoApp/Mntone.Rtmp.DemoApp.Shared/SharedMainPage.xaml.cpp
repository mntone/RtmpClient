#include "pch.h"
#include "MainPage.xaml.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::System::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Mntone::Rtmp::Client;
using namespace Mntone::Rtmp::DemoApp;

MainPage::MainPage()
{
	InitializeComponent();

	myAppContext_ = safe_cast<App^>( App::Current );
}

void MainPage::OnPageUnloaded( Object^ sender, RoutedEventArgs^ e )
{
	CloseClient();
}

void MainPage::OnButtonClicked( Object^ sender, RoutedEventArgs^ e )
{
	CloseClient();

	foregroundElement->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	progressRing->Visibility = ::Visibility::Visible;
	progressRing->IsActive = true;

	auto uri = uriTextBox->Text;

	client_ = ref new SimpleVideoClient();
	startedEventToken_ = client_->Started += ref new EventHandler<SimpleVideoClientStartedEventArgs^>( this, &MainPage::OnStarted );
	stoppedEventToken_ = client_->Stopped += ref new EventHandler<SimpleVideoClientStoppedEventArgs^>( this, &MainPage::OnStopped );
	client_->ConnectAsync( ref new Uri( uri ) );

	auto smtc = SystemMediaTransportControls::GetForCurrentView();
	smtc->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls^, SystemMediaTransportControlsButtonPressedEventArgs^>( this, &MainPage::OnMediaButtonPressed );
	smtc->IsEnabled = true;
	smtc->IsPlayEnabled = true;
	smtc->IsPauseEnabled = true;
	smtc->IsStopEnabled = true;
	smtc->PlaybackStatus = MediaPlaybackStatus::Playing;
}

void MainPage::OnStarted( Object^ sender, SimpleVideoClientStartedEventArgs^ args )
{
	Dispatcher->RunAsync( CoreDispatcherPriority::High, ref new DispatchedHandler( [=]
	{
		mediaElement->SetMediaStreamSource( args->MediaStreamSource );
		mediaElement->CurrentStateChanged += ref new RoutedEventHandler( this, &MainPage::OnCurrentStateChanged );
		mediaElement->MediaEnded += ref new RoutedEventHandler( this, &MainPage::OnMediaEnded );
		mediaElement->Play();

		myAppContext_->DisplayRequestActive();
	} ) );
}

void MainPage::OnStopped( Object^ sender, SimpleVideoClientStoppedEventArgs^ args )
{
	myAppContext_->DisplayRequestRelease();
}

void MainPage::OnCurrentStateChanged( Object^ sender, RoutedEventArgs^ e )
{
	switch( mediaElement->CurrentState )
	{
	case MediaElementState::Playing:
		progressRing->Visibility = ::Visibility::Collapsed;
		progressRing->IsActive = false;
		break;
	case MediaElementState::Buffering:
		progressRing->Visibility = ::Visibility::Visible;
		progressRing->IsActive = true;
		break;
	}
}

void MainPage::OnMediaEnded( Object^ sender, RoutedEventArgs^ e )
{
	foregroundElement->Visibility = ::Visibility::Visible;
	CloseClient();
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