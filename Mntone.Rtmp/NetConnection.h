#pragma once
#include "Command/IRtmpCommand.h"
#include "limit_type.h"
#include "rtmp_packet.h"
#include "RtmpUri.h"
#include "Connection.h"
#include "NetStatusUpdatedEventArgs.h"
#include "NetConnectionClosedEventArgs.h"
#include "UserControlMessageEventType.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public delegate void RtmpDynamicHandler( Mntone::Data::Amf::AmfArray^ data );

	ref class NetStream;

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WIN8 )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::MTA )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetConnection sealed
	{
	public:
		NetConnection( void );
		virtual ~NetConnection( void );

		// Connect
		void Connect( Windows::Foundation::Uri^ uri );
		void Connect( Windows::Foundation::Uri^ uri, Command::IRtmpCommand^ connectCommand );
		[Windows::Foundation::Metadata::DefaultOverload] void Connect( RtmpUri^ uri );
		[Windows::Foundation::Metadata::DefaultOverload] void Connect( RtmpUri^ uri, Command::IRtmpCommand^ connectCommand );

		// This method is not supported.
		//void Call( void );

	internal:
		// Send
		void SendWithAction( uint32 streamId, Mntone::Data::Amf::AmfArray^ amf );

		// Utilites
		void AttachNetStream( NetStream^ stream );
		void UnattachNetStream( NetStream^ stream );

	private:
		// Connect
		void __Connect( Command::IRtmpCommand^ connectCommand );

		// Receive
		void Receive( void );
		void __Receive( void );
		void OnMessage( const rtmp_packet packet, std::vector<uint8> data );
		void OnNetworkMessage( const rtmp_packet packet, std::vector<uint8> data );
		void OnUserControlMessage( const rtmp_packet packet, std::vector<uint8> data );
		void OnCommandMessageAmf0( const rtmp_packet packet, std::vector<uint8> data );
		void OnCommandMessageAmf3( const rtmp_packet packet, std::vector<uint8> data );
		void OnCommandMessage( Mntone::Data::Amf::AmfArray^ amf );

		// Send
		void SetChunkSize( uint32 chunkSize );
		void AbortMessage( uint32 chunkStreamId );
		void Acknowledgement( uint32 sequenceNumber );
		void WindowAcknowledgementSize( uint32 acknowledgementWindowSize );
		void SetPeerBandWidth( uint32 windowSize, limit_type type );

		void SetBufferLength( const uint32 streamId, const uint32 bufferLength );
		void PingResponse( const uint32 timestamp );
		void UserControlMessageEvent( UserControlMessageEventType type, std::vector<uint8> data );

		void SendWithNetwork( const type_id_type type, const std::vector<uint8> data );
		void SendWithAction( Mntone::Data::Amf::AmfArray^ amf );

		void Send( const rtmp_packet packet, const std::vector<uint8> data, const bool isFormatTypeZero = false );
		void __Send( rtmp_packet packet, const std::vector<uint8> data, const bool isFormatTypeZero = false );
		std::vector<uint8> CreateHeader( rtmp_packet packet, bool isFormatTypeZero );

	public:
		//event Windows::Foundation::TypedEventHandler<NetConnection^, NetStatusUpdatedEventArgs^>^ StatusUpdated;
		//event Windows::Foundation::TypedEventHandler<NetConnection^, NetConnectionClosedEventArgs^>^ Closed;
		event Windows::Foundation::EventHandler<NetStatusUpdatedEventArgs^>^ StatusUpdated;
		event Windows::Foundation::EventHandler<NetConnectionClosedEventArgs^>^ Closed;

	public:
		property RtmpUri^ Uri
		{
			RtmpUri^ get( void ) { return _Uri; }
		}
		// Now, this property is read-only; however, change read-write when Mntone::Data::Amf implements amf3 format.
		property Mntone::Data::Amf::AmfEncodingType DefaultEncodingType
		{
			Mntone::Data::Amf::AmfEncodingType get( void ) { return _DefaultEncodingType; }
			//void set( Mntone::Data::Amf::AmfEncodingType value ) { _DefaultEncodingType = value; }
		}
		property Windows::Foundation::Collections::IMapView<Platform::String^, RtmpDynamicHandler^>^ Client
		{
			Windows::Foundation::Collections::IMapView<Platform::String^, RtmpDynamicHandler^>^ get( void ) { return _Client; }
			void set( Windows::Foundation::Collections::IMapView<Platform::String^, RtmpDynamicHandler^>^ value ) { _Client = value; }
		}

	internal:
		int64 _startTime;
		Connection^ _connection;
		Mntone::Data::Amf::AmfEncodingType _DefaultEncodingType;

	private:
		RtmpUri^ _Uri;
		Command::IRtmpCommand^ _connectCommand;
		Windows::Foundation::Collections::IMapView<Platform::String^, RtmpDynamicHandler^>^ _Client;

		uint32 _latestTransactionId;
		std::map<uint32, NetStream^> _netStreamTemporary;

		std::map<uint32, NetStream^> _bindingNetStream;

		std::vector<uint8> _rxHeaderBuffer;
		std::map<uint16, std::shared_ptr<rtmp_packet>> _rxBakPackets, _txBakPackets;
		uint32 _rxWindowSize, _txWindowSize;
		uint32 _rxChunkSize, _txChunkSize;
	};

} }