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
		NetStream( void );
		NetStream( NetConnection^ connection );
		virtual ~NetStream( void );

		void Attach( NetConnection^ connection );

		void Play( Platform::String^ streamName );
		void Play( Platform::String^ streamName, int32 start );
		void Play( Platform::String^ streamName, int32 start, int32 duration );

		void Pause( void );
		void Resume( void );
		void Seek( uint32 offset );

	internal:
		void __Attached( void );

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
		void SendWithAction( Mntone::Data::Amf::AmfArray^ amf );

		void AnalysisAvc( const rtmp_packet packet, std::vector<uint8> data, NetStreamVideoReceivedEventArgs^& args );
			
	public:
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamAttachedEventArgs^>^ Attached;
		event Windows::Foundation::EventHandler<NetStatusUpdatedEventArgs^>^ StatusUpdated;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamAudioStartedEventArgs^>^ AudioStarted;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamAudioReceivedEventArgs^>^ AudioReceived;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamVideoStartedEventArgs^>^ VideoStarted;
		event Windows::Foundation::TypedEventHandler<NetStream^, NetStreamVideoReceivedEventArgs^>^ VideoReceived;
		
	internal:
		NetConnection^ _parent;
		uint32 _streamId;

	private:
		bool _audioInfoEnabled;
		AudioInfo^ _audioInfo;

		bool _videoInfoEnabled;
		VideoInfo^ _videoInfo;

		// for Avc
		uint8 _length_size_minus_one;
	};

} }