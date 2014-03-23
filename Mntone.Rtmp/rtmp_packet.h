#pragma once
#include "rtmp_header.h"

namespace mntone { namespace rtmp {

	class rtmp_packet final
	{
	public:
		rtmp_packet() = delete;
		rtmp_packet( const rtmp_packet& ) = delete;
		rtmp_packet( rtmp_packet&& ) = default;

		rtmp_packet& operator=( const rtmp_packet& rhs ) = delete;
		rtmp_packet& operator=( rtmp_packet&& ) = default;

		explicit rtmp_packet( uint16 chunk_stream_id )
			: header_( chunk_stream_id )
			, temporary_length_( 0 )
		{ }

	public:
		rtmp_header header_;
		uint32 temporary_length_;
		std::unique_ptr<std::vector<uint8>> body_;
	};

} }
