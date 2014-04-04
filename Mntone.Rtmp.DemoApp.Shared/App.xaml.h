#pragma once
#include "App.g.h"

namespace Mntone { namespace Rtmp { namespace DemoApp {

	ref class App sealed
	{
	internal:
		App();

	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	private:
#if WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
		Windows::UI::Xaml::Media::Animation::TransitionCollection^ _transitions;
		Windows::Foundation::EventRegistrationToken _firstNavigatedToken;

		void RootFrame_FirstNavigated( Platform::Object^ sender, Windows::UI::Xaml::Navigation::NavigationEventArgs^ e );
#endif

		void OnSuspending( Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e );
	};

} } }
