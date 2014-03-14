#pragma once

namespace Mntone { namespace Rtmp {

	delegate void ConnectionCallbackHandler( Windows::Storage::Streams::IBuffer^ result );

	ref class Connection sealed
	{
	internal:
		Connection();

		Concurrency::task<void> ConnectAsync( Platform::String^ host, Platform::String^ port );

		void Read( const uint32 length, ConnectionCallbackHandler^ callbackFunction );

		Concurrency::task<void> Write( const uint8* const data, const size_t length );

	private:
		void CloseImpl();
		void ContinuousRead( Windows::Storage::Streams::IBuffer^ data, const uint32 length, ConnectionCallbackHandler^ callbackFunction );

	internal:
		event Windows::Foundation::TypedEventHandler<Connection^, Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^>^ ReadOperationChanged;

	internal:
		property bool IsInitialized
		{
			bool get() { return IsInitialized_; }
		}

	private:
		bool IsInitialized_;
		Windows::Networking::Sockets::StreamSocket^ streamSocket_;
		Windows::Storage::Streams::DataWriter^ dataWriter_;
	};

} }
