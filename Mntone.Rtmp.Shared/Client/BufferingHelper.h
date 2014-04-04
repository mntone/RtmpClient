#pragma once

namespace Mntone { namespace Rtmp {

	ref class NetStream;
	ref class NetStreamAudioReceivedEventArgs;
	ref class NetStreamVideoReceivedEventArgs;
	
namespace Client {

	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::Both )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class BufferingHelper sealed
	{
	public:
		BufferingHelper( NetStream^ stream );

		void Stop();

		Windows::Foundation::IAsyncOperation<Windows::Media::Core::MediaStreamSample^>^ GetAudioAsync();
		Windows::Foundation::IAsyncOperation<Windows::Media::Core::MediaStreamSample^>^ GetVideoAsync();

	private:
		~BufferingHelper();

		void OnAudioReceived( Platform::Object^ sender, NetStreamAudioReceivedEventArgs^ args );
		void OnVideoReceived( Platform::Object^ sender, NetStreamVideoReceivedEventArgs^ args );

	private:
		NetStream^ stream_;
		Windows::Foundation::EventRegistrationToken audioReceivedEventToken_, videoReceivedEventToken_;

		bool isEnable_;
		mutable std::mutex audioMutex_, videoMutex_;
		std::condition_variable audioConditionVariable_, videoConditionVariable_;
		std::queue<Windows::Media::Core::MediaStreamSample^> audioBuffer_;
		std::queue<Windows::Media::Core::MediaStreamSample^> videoBuffer_;
	};

} } }