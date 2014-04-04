#pragma once
#include "rtmp_packet.h"
#include "NetConnection.h"
#include "NetStreamAttachedEventArgs.h"
#include "NetStatusUpdatedEventArgs.h"
#include "NetStreamAudioStartedEventArgs.h"
#include "NetStreamAudioReceivedEventArgs.h"
#include "NetStreamVideoStartedEventArgs.h"
#include "NetStreamVideoReceivedEventArgs.h"
#include "Media/avc_decoder_configuration_record.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStream sealed
	{
	public:
		NetStream();

		Windows::Foundation::IAsyncAction^ AttachAsync( NetConnection^ connection );

		Windows::Foundation::IAsyncAction^ PlayAsync( Platform::String^ streamName );
		Windows::Foundation::IAsyncAction^ PlayAsync( Platform::String^ streamName, float64 start );
		Windows::Foundation::IAsyncAction^ PlayAsync( Platform::String^ streamName, float64 start, float64 duration );
		Windows::Foundation::IAsyncAction^ PlayAsync( Platform::String^ streamName, float64 start, float64 duration, int16 reset );

		Windows::Foundation::IAsyncAction^ PauseAsync( float64 position );
		Windows::Foundation::IAsyncAction^ ResumeAsync( float64 position );
		Windows::Foundation::IAsyncAction^ SeekAsync( float64 offset );

	internal:
		void AttachedImpl();
		void DetachedImpl();

		void OnMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnAudioMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnVideoMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnDataMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnCommandMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnAggregateMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );

	private:
		~NetStream();

		Concurrency::task<void> SendActionAsync( Mntone::Data::Amf::AmfArray^ amf );

		void AnalysisAvc( mntone::rtmp::rtmp_header header, std::vector<uint8> data, NetStreamVideoReceivedEventArgs^& args );
			
	public:
		event Windows::Foundation::EventHandler<NetStreamAttachedEventArgs^>^ Attached;
		event Windows::Foundation::EventHandler<NetStatusUpdatedEventArgs^>^ StatusUpdated;
		event Windows::Foundation::EventHandler<NetStreamAudioStartedEventArgs^>^ AudioStarted;
		event Windows::Foundation::EventHandler<NetStreamAudioReceivedEventArgs^>^ AudioReceived;
		event Windows::Foundation::EventHandler<NetStreamVideoStartedEventArgs^>^ VideoStarted;
		event Windows::Foundation::EventHandler<NetStreamVideoReceivedEventArgs^>^ VideoReceived;

	internal:
		NetConnection^ parent_;
		uint32 streamId_;

	private:
		bool audioEnabled_, audioInfoEnabled_;
		Media::AudioInfo^ audioInfo_;

		bool videoEnabled_, videoInfoEnabled_;
		Media::VideoInfo^ videoInfo_;
		uint16 videoDataRate_;

		// for Avc
		uint8 lengthSizeMinusOne_;

		// for AAC
		uint32 samplingRate_;
	};

} }