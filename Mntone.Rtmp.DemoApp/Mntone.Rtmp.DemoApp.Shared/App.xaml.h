#pragma once
#include "App.g.h"

namespace Mntone { namespace Rtmp { namespace DemoApp {

	ref class App sealed
	{
	internal:
		App();

		void DisplayRequestActive();
		void DisplayRequestRelease();

	protected:
		virtual void OnLaunched( ::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e ) override sealed;

	private:
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		void OnFirstNavigated( Platform::Object^ sender, Windows::UI::Xaml::Navigation::NavigationEventArgs^ e );
#endif

		void OnSuspending( ::Platform::Object^ sender, ::Windows::ApplicationModel::SuspendingEventArgs^ e );

	private:
		static uint32 displayRequestRef_;
		static ::Windows::System::Display::DisplayRequest^ displayRequest_;

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		Windows::UI::Xaml::Media::Animation::TransitionCollection^ transitions_;
		Windows::Foundation::EventRegistrationToken firstNavigatedToken_;
#endif
	};

} } }
