#pragma once
#include "Command/NetConnectionConnectCommand.h"
#include "Command/NetConnectionCallCommand.h"
#include "limit_type.h"
#include "rtmp_packet.h"
#include "RtmpUri.h"
#include "Connection.h"
#include "NetStatusUpdatedEventArgs.h"
#include "NetConnectionClosedEventArgs.h"
#include "NetConnectionCallbackEventArgs.h"
#include "UserControlMessageEventType.h"

namespace Mntone { namespace Rtmp {

	delegate void HandshakeCallbackHandler();

	ref class NetStream;

	[Windows::Foundation::Metadata::DualApiPartition( version = NTDDI_WINBLUE )]
	[Windows::Foundation::Metadata::MarshalingBehavior( Windows::Foundation::Metadata::MarshalingType::Agile )]
	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::Both )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetConnection sealed
	{
	public:
		NetConnection();

		// Connect
		Windows::Foundation::IAsyncAction^ ConnectAsync( Windows::Foundation::Uri^ uri );
		Windows::Foundation::IAsyncAction^ ConnectAsync( Windows::Foundation::Uri^ uri, Command::NetConnectionConnectCommand^ command );
		[Windows::Foundation::Metadata::DefaultOverload] Windows::Foundation::IAsyncAction^ ConnectAsync( RtmpUri^ uri );
		[Windows::Foundation::Metadata::DefaultOverload] Windows::Foundation::IAsyncAction^ ConnectAsync( RtmpUri^ uri, Command::NetConnectionConnectCommand^ connectCommand );

		// Call
		Windows::Foundation::IAsyncAction^ CallAsync( Command::NetConnectionCallCommand^ command );

	internal:
		// Send
		Concurrency::task<void> SendActionAsync( uint32 streamId, Mntone::Data::Amf::AmfArray^ amf );

		// Utilites
		Concurrency::task<void> AttachNetStreamAsync( NetStream^ stream );
		void UnattachNetStream( NetStream^ stream );

	private:
		// Close
		void CloseImpl();

		// Handshake
		void Handshake( HandshakeCallbackHandler^ callbackFunction );

		// Receive
		void OnReadOperationChanged( Connection^ sender, Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^ operation );
		void Receive();
		void ReceiveContinueImpl( Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^ operation );
		void ReceiveHeader1Impl( const std::vector<uint8> result );
		void ReceiveHeader2Impl( const uint8 format_type, const uint16 chunk_stream_id );
		void ReceiveBodyImpl( const std::shared_ptr<mntone::rtmp::rtmp_packet> packet );
		void ReceiveCallbackImpl( const std::shared_ptr<mntone::rtmp::rtmp_packet> packet, const std::vector<uint8> result );

		void OnMessage( const mntone::rtmp::rtmp_packet packet, std::vector<uint8> data );
		void OnNetworkMessage( const mntone::rtmp::rtmp_packet packet, std::vector<uint8> data );
		void OnUserControlMessage( const mntone::rtmp::rtmp_packet packet, std::vector<uint8> data );
		void OnCommandMessage( const mntone::rtmp::rtmp_packet packet, std::vector<uint8> data );

		// Send
		Concurrency::task<void> SetChunkSizeAsync( const uint32 chunkSize );
		Concurrency::task<void> AbortMessageAsync( const uint32 chunkStreamId );
		Concurrency::task<void> AcknowledgementAsync( const uint32 sequenceNumber );
		Concurrency::task<void> WindowAcknowledgementSizeAsync( const uint32 acknowledgementWindowSize );
		Concurrency::task<void> SetPeerBandWidthAsync( const uint32 windowSize, const mntone::rtmp::limit_type type );

		Concurrency::task<void> SetBufferLengthAsync( const uint32 streamId, const uint32 bufferLength );
		Concurrency::task<void> PingResponseAsync( const uint32 timestamp );
		Concurrency::task<void> UserControlMessageEventAsync( UserControlMessageEventType type, std::vector<uint8> data );

		Concurrency::task<void> SendNetworkAsync( const mntone::rtmp::type_id_type type, const std::vector<uint8> data );
		Concurrency::task<void> SendActionAsync( Mntone::Data::Amf::AmfArray^ amf );

		Concurrency::task<void> SendAsync( mntone::rtmp::rtmp_packet packet, const std::vector<uint8> data, const bool isFormatTypeZero = false );
		std::vector<uint8> CreateHeader( mntone::rtmp::rtmp_packet packet, bool isFormatTypeZero );

	public:
		event Windows::Foundation::EventHandler<NetStatusUpdatedEventArgs^>^ StatusUpdated;
		event Windows::Foundation::TypedEventHandler<NetConnection^, NetConnectionClosedEventArgs^>^ Closed;
		event Windows::Foundation::TypedEventHandler<NetConnection^, NetConnectionCallbackEventArgs^>^ Callback;

	public:
		property RtmpUri^ Uri
		{
			RtmpUri^ get() { return Uri_; }
		}

	private:
		int64 startTime_;
		RtmpUri^ Uri_;
		Connection^ connection_;
		Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^ receiveOperation_;

		uint32 latestTransactionId_;
		std::unordered_map<uint32, NetStream^> netStreamTemporary_;
		std::unordered_map<uint32, NetStream^> bindingNetStream_;

		std::vector<uint8> rxHeaderBuffer_;
		std::unordered_map<uint16, std::shared_ptr<mntone::rtmp::rtmp_packet>> rxBakPackets_, txBakPackets_;
		uint32 rxWindowSize_, txWindowSize_;
		uint32 rxChunkSize_, txChunkSize_;
	};

} }