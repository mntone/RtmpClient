#include "pch.h"
#include "MainPage.xaml.h"

using namespace Mntone::Rtmp::DemoApp;
using namespace Mntone::Rtmp::Client;
namespace WUIX = Windows::UI::Xaml;

MainPage::MainPage()
{
	InitializeComponent();
}

void MainPage::OnPageUnloaded( Platform::Object^ sender, WUIX::RoutedEventArgs^ e )
{
	if( client_ != nullptr )
		delete client_;
}

void MainPage::OnButtonClicked( Platform::Object^ sender, WUIX::RoutedEventArgs^ e )
{
	auto uri = Uri->Text;

	client_ = ref new Mntone::Rtmp::Client::SimpleVideoClient();
	client_->Started += ref new Windows::Foundation::EventHandler<SimpleVideoClientStartedEventArgs^>( this, &MainPage::OnStarted );
	client_->Connect( ref new Windows::Foundation::Uri( uri ) );
}

void MainPage::OnStarted( Platform::Object^ sender, SimpleVideoClientStartedEventArgs^ args )
{
	mediaElement->SetMediaStreamSource( args->MediaStreamSource );
	mediaElement->Play();
}

// // for backgrounding test
//void MainPage::OnButtonPressed( Windows::Media::SystemMediaTransportControls^ sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^ e )
//{
//	if( e->Button == Windows::Media::SystemMediaTransportControlsButton::Pause || e->Button == Windows::Media::SystemMediaTransportControlsButton::Stop )
//		Dispatcher->RunAsync( Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler( [] { } ) );
//}
//
//	Dispatcher->RunAsync( Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler( [this]()
//	{
//
//		auto smtc = Windows::Media::SystemMediaTransportControls::GetForCurrentView();
//		smtc->ButtonPressed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::SystemMediaTransportControls^, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^>( this, &MainPage::OnButtonPressed );
//		smtc->IsEnabled = true;
//		smtc->IsPlayEnabled = true;
//		smtc->IsStopEnabled = true;
//		smtc->PlaybackStatus = Windows::Media::MediaPlaybackStatus::Playing;
//
//	} ) );
