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

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class NetConnection sealed
	{
	public:
		NetConnection();

		// Connect
		[Windows::Foundation::Metadata::Overload( "ConnectWithDefaultUriAsync" )]
		Windows::Foundation::IAsyncAction^ ConnectAsync( Windows::Foundation::Uri^ uri );

		[Windows::Foundation::Metadata::Overload( "ConnectWithDefaultUriAndCommandAsync" )]
		Windows::Foundation::IAsyncAction^ ConnectAsync( Windows::Foundation::Uri^ uri, Command::NetConnectionConnectCommand^ command );

		[Windows::Foundation::Metadata::Overload( "ConnectAsync" )]
		[Windows::Foundation::Metadata::DefaultOverload]
		Windows::Foundation::IAsyncAction^ ConnectAsync( RtmpUri^ uri );

		[Windows::Foundation::Metadata::Overload( "ConnectWithCommandAsync" )]
		[Windows::Foundation::Metadata::DefaultOverload]
		Windows::Foundation::IAsyncAction^ ConnectAsync( RtmpUri^ uri, Command::NetConnectionConnectCommand^ connectCommand );

		// Call
		Windows::Foundation::IAsyncAction^ CallAsync( Command::NetConnectionCallCommand^ command );

	internal:
		// Send
		Concurrency::task<void> SendActionAsync( uint32 streamId, Mntone::Data::Amf::AmfArray^ amf );

		// Utilites
		Concurrency::task<void> AttachNetStreamAsync( NetStream^ stream );
		void UnattachNetStream( NetStream^ stream );

	private:
		~NetConnection();

		// Close
		void CloseImpl();

		// Handshake
		void Handshake( HandshakeCallbackHandler^ callbackFunction );

		// Receive
		void OnReadOperationChanged( Connection^ sender, Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^ operation );
		void Receive();
		void ReceiveContinueImpl( Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^ operation );
		void ReceiveHeader1Impl( Windows::Storage::Streams::IBuffer^ result );
		void ReceiveHeader2Impl( const uint8 format_type, const uint16 chunk_stream_id );
		void ReceiveBodyImpl( std::shared_ptr<mntone::rtmp::rtmp_packet> packet );
		void ReceiveCallbackImpl( mntone::rtmp::rtmp_header header, std::vector<uint8> data );

		void OnMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );

		void OnNetworkMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnSetChunkSize( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnAbortMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnAcknowledgement( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnUserControlMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnWindowAcknowledgementSize( mntone::rtmp::rtmp_header header, std::vector<uint8> data );
		void OnSetPeerBandwidthMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );

		void OnCommandMessage( mntone::rtmp::rtmp_header header, std::vector<uint8> data );

		// Send
		Concurrency::task<void> SetChunkSizeAsync( const int32 chunkSize );
		Concurrency::task<void> AbortMessageAsync( const uint32 chunkStreamId );
		Concurrency::task<void> AcknowledgementAsync( const uint32 sequenceNumber );
		Concurrency::task<void> WindowAcknowledgementSizeAsync( const uint32 acknowledgementWindowSize );
		Concurrency::task<void> SetPeerBandWidthAsync( const uint32 windowSize, const mntone::rtmp::limit_type type );

		Concurrency::task<void> SetBufferLengthAsync( const uint32 streamId, const uint32 bufferLength );
		Concurrency::task<void> PingResponseAsync( const uint32 timestamp );
		Concurrency::task<void> UserControlMessageEventAsync( UserControlMessageEventType type, std::vector<uint8> data );

		Concurrency::task<void> SendNetworkAsync( const mntone::rtmp::type_id_type type, std::vector<uint8> data );
		Concurrency::task<void> SendActionAsync( Mntone::Data::Amf::AmfArray^ amf );

		Concurrency::task<void> SendAsync( mntone::rtmp::rtmp_header header, std::vector<uint8> data, const uint8 forceFormatType = 255, size_t temporary_length = 0 );
		std::vector<uint8> CreateHeader( mntone::rtmp::rtmp_header header, uint8_t forceFormatType );

	public:
		event Windows::Foundation::EventHandler<NetStatusUpdatedEventArgs^>^ StatusUpdated;
		event Windows::Foundation::EventHandler<NetConnectionClosedEventArgs^>^ Closed;
		event Windows::Foundation::EventHandler<NetConnectionCallbackEventArgs^>^ Callback;

	public:
		property RtmpUri^ Uri
		{
			RtmpUri^ get() { return Uri_; }
		}

	private:
		int64 startTime_;
		RtmpUri^ Uri_;
		Connection^ connection_;
		Windows::Foundation::EventRegistrationToken readOperationEventToken_;
		Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^ receiveOperation_;

		uint32 latestTransactionId_;
		std::unordered_map<uint32, NetStream^> netStreamTemporary_;
		std::unordered_map<uint32, NetStream^> bindingNetStream_;

		std::vector<uint8> rxHeaderBuffer_;
		std::unordered_map<uint16, std::shared_ptr<mntone::rtmp::rtmp_packet>> rxBakPackets_;
		std::unordered_map<uint16, std::shared_ptr<mntone::rtmp::rtmp_header>> txBakHeaders_;

		int32 rxChunkSize_, txChunkSize_;
		uint32 rxWindowSize_, txWindowSize_;
		mntone::rtmp::limit_type rxLimitType_, txLimitType_;
	};

} }