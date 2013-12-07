#pragma once
#include "rtmp_packet.h"
#include "NetConnection.h"
#include "NetStreamAttachedEventArgs.h"
#include "NetStatusUpdatedEventArgs.h"
#include "NetStreamAudioStartedEventArgs.h"
#include "NetStreamAudioReceivedEventArgs.h"
#include "NetStreamVideoStartedEventArgs.h"
#include "NetStreamVideoReceivedEventArgs.h"
#include "avc_decoder_configuration_record.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WIN8 )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::MTA )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetStream sealed
	{
	public:
		NetStream();
		virtual ~NetStream();

		void Attach( NetConnection^ connection );

		void Play( Platform::String^ streamName );
		void Play( Platform::String^ streamName, int32 start );
		void Play( Platform::String^ streamName, int32 start, int32 duration );

		void Pause();
		void Resume();
		void Seek( uint32 offset );

	internal:
		void AttachedImpl();

		void OnMessage( const rtmp_packet packet, std::vector<uint8> data );
		void OnAudioMessage( const rtmp_packet packet, std::vector<uint8> data );
		void OnVideoMessage( const rtmp_packet packet, std::vector<uint8> data );
		void OnDataMessageAmf0( const rtmp_packet packet, std::vector<uint8> data );
		void OnDataMessageAmf3( const rtmp_packet packet, std::vector<uint8> data );
		void OnDataMessage( Mntone::Data::Amf::AmfArray^ amf );
		void OnCommandMessageAmf0( const rtmp_packet packet, std::vector<uint8> data );
		void OnCommandMessageAmf3( const rtmp_packet packet, std::vector<uint8> data );
		void OnCommandMessage( Mntone::Data::Amf::AmfArray^ amf );

	private:
		Concurrency::task<void> SendActionAsync( Mntone::Data::Amf::AmfArray^ amf );

		void AnalysisAvc( const rtmp_packet packet, std::vector<uint8> data, NetStreamVideoReceivedEventArgs^& args );
			
	public:
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamAttachedEventArgs^>^ Attached;
		event Windows::Foundation::EventHandler<NetStatusUpdatedEventArgs^>^ StatusUpdated;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamAudioStartedEventArgs^>^ AudioStarted;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamAudioReceivedEventArgs^>^ AudioReceived;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamVideoStartedEventArgs^>^ VideoStarted;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamVideoReceivedEventArgs^>^ VideoReceived;
		
	internal:
		NetConnection^ parent_;
		uint32 streamId_;

	private:
		bool audioInfoEnabled_;
		AudioInfo^ audioInfo_;

		bool videoInfoEnabled_;
		VideoInfo^ videoInfo_;

		// for Avc
		uint8 lengthSizeMinusOne_;
	};

} }