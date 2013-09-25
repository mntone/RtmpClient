#include "pch.h"
#include "MainPage.xaml.h"

using namespace Mntone::Rtmp::DemoApp;

MainPage::MainPage()
{
	InitializeComponent();
}

void MainPage::OnButtonClicked( Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e )
{
	auto uri = Uri->Text;

	_client = ref new Mntone::Rtmp::Client::SimpleVideoClient();
	_client->Started += ref new Windows::Foundation::EventHandler<Mntone::Rtmp::Client::SimpleVideoClientStartedEventArgs ^>( this, &MainPage::OnStarted );
	_client->Connect( ref new Windows::Foundation::Uri( uri ) );
}

void MainPage::OnStarted( Platform::Object^ sender, Mntone::Rtmp::Client::SimpleVideoClientStartedEventArgs^ args )
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
//void MainPage::OnStarted( Mntone::Rtmp::LiveRtmpClientHelper^ sender/*, Rtmp::LiveRtmpClientHelperLiveStartedEventArgs^ args*/ )
//{
//	Dispatcher->RunAsync( WUC::CoreDispatcherPriority::High, ref new WUC::DispatchedHandler( [this]()
//	{
//
//		auto smtc = WM::SystemMediaTransportControls::GetForCurrentView();
//		smtc->ButtonPressed += ref new WF::TypedEventHandler<WM::SystemMediaTransportControls^, WM::SystemMediaTransportControlsButtonPressedEventArgs^>( this, &MainPage::OnButtonPressed );
//		smtc->IsEnabled = true;
//		smtc->IsPlayEnabled = true;
//		smtc->IsPauseEnabled = true;
//		smtc->IsStopEnabled = true;
//		smtc->PlaybackStatus = WM::MediaPlaybackStatus::Playing;
//
//	} ) );
//}
