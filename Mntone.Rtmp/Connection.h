#pragma once

namespace Mntone { namespace Rtmp {

	ref class Connection sealed
	{
	internal:
		Connection();

	public:
		virtual ~Connection();

	internal:
		Concurrency::task<void> ConnectAsync( Platform::String^ host, Platform::String^ port );

		uint32 TryRead( uint8* const data, const size_t length );

		void Read( uint8* const data, const size_t length );
		void Read( std::vector<uint8>& data );
		void Read( std::vector<uint8>& data, const size_t length );

		void Write( const uint8* const data, const size_t length );
		void Write( const std::vector<uint8>& data );
		void Write( const std::vector<uint8>& data, const size_t length );

	internal:
		property bool IsInitialized
		{
			bool get() { return IsInitialized_; }
		}

	private:
		bool IsInitialized_;
		Windows::Networking::Sockets::StreamSocket^ streamSocket_;
		Windows::Storage::Streams::DataReader^ dataReader_;
		Windows::Storage::Streams::DataWriter^ dataWriter_;
	};

} }
