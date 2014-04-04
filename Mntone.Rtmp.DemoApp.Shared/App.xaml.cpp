#include "pch.h"
#include "MainPage.xaml.h"

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media::Animation;
using namespace Windows::UI::Xaml::Navigation;
using namespace Mntone::Rtmp::DemoApp;

App::App()
{
	InitializeComponent();
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
			_transitions = ref new TransitionCollection();
			for( auto transition : rootFrame->ContentTransitions )
			{
				_transitions->Append( transition );
			}
		}

		rootFrame->ContentTransitions = nullptr;
		_firstNavigatedToken = rootFrame->Navigated += ref new NavigatedEventHandler( this, &App::RootFrame_FirstNavigated );
#endif

		if( !rootFrame->Navigate( TypeName( MainPage::typeid ), e->Arguments ) )
		{
			throw ref new FailureException( "Failed to create initial page" );
		}
	}
	Window::Current->Activate();
}

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
void App::RootFrame_FirstNavigated( Object^ sender, NavigationEventArgs^ e )
{
	auto rootFrame = dynamic_cast<Frame^>( sender );
	if( rootFrame == nullptr )
	{
		return;
	}

	TransitionCollection^ newTransitions;
	if( _transitions == nullptr )
	{
		newTransitions = ref new TransitionCollection();
		newTransitions->Append( ref new NavigationThemeTransition() );
	}
	else
	{
		newTransitions = _transitions;
	}

	rootFrame->ContentTransitions = newTransitions;

	rootFrame->Navigated -= _firstNavigatedToken;
}
#endif