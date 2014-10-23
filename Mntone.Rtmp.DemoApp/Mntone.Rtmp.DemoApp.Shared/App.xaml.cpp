#include "pch.h"
#include "MainPage.xaml.h"

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::System::Display;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media::Animation;
using namespace Windows::UI::Xaml::Navigation;
using namespace Mntone::Rtmp::DemoApp;

uint32 App::displayRequestRef_ = 0;
DisplayRequest^ App::displayRequest_ = nullptr;

App::App()
{
	InitializeComponent();
}

void App::DisplayRequestActive()
{
	if( displayRequest_ == nullptr )
	{
		displayRequest_ = ref new DisplayRequest();
	}

	if( ++displayRequestRef_ == 1 )
	{
		displayRequest_->RequestActive();
	}
}

void App::DisplayRequestRelease()
{
	if( displayRequest_ == nullptr )
	{
		displayRequest_ = ref new DisplayRequest();
	}

	if( displayRequestRef_ != 0 )
	{
		if( --displayRequestRef_ == 0 )
		{
			displayRequest_->RequestRelease();
		}
	}
}

void App::OnLaunched( LaunchActivatedEventArgs^ e )
{
#if _DEBUG
	if( IsDebuggerPresent() )
	{
		DebugSettings->EnableFrameRateCounter = true;
	}
#endif

	auto rootFrame = dynamic_cast<Frame^>( Window::Current->Content );
	if( rootFrame == nullptr )
	{
		rootFrame = ref new Frame();
		rootFrame->CacheSize = 1;
		Window::Current->Content = rootFrame;
	}
	if( rootFrame->Content == nullptr )
	{
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		if( rootFrame->ContentTransitions != nullptr )
		{
			transitions_ = ref new TransitionCollection();
			for( auto transition : rootFrame->ContentTransitions )
			{
				transitions_->Append( transition );
			}
		}

		rootFrame->ContentTransitions = nullptr;
		firstNavigatedToken_ = rootFrame->Navigated += ref new NavigatedEventHandler( this, &App::OnFirstNavigated );
#endif

		if( !rootFrame->Navigate( TypeName( MainPage::typeid ), e->Arguments ) )
		{
			throw ref new FailureException( "Failed to create initial page" );
		}
	}
	Window::Current->Activate();
}

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
void App::OnFirstNavigated( Object^ sender, NavigationEventArgs^ e )
{
	auto rootFrame = dynamic_cast<Frame^>( sender );
	if( rootFrame == nullptr )
	{
		return;
	}

	TransitionCollection^ newTransitions;
	if( transitions_ == nullptr )
	{
		newTransitions = ref new TransitionCollection();
		newTransitions->Append( ref new NavigationThemeTransition() );
	}
	else
	{
		newTransitions = transitions_;
	}

	rootFrame->ContentTransitions = newTransitions;

	rootFrame->Navigated -= firstNavigatedToken_;
}
#endif