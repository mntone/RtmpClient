#include "pch.h"
#include "NetStream.h"

using namespace mntone::rtmp;
using namespace mntone::rtmp::media;
using namespace Mntone::Rtmp;
using namespace Mntone::Rtmp::Media;

void NetStream::AnalysisAvc( const rtmp_packet packet, std::vector<uint8> data, NetStreamVideoReceivedEventArgs^& args )
{
	if( data.size() < 5 )
	{
		return;
	}

	// AVC NALU
	if( data[1] == 0x01 )
	{
		args->Info = videoInfo_;

		int64 composition_time_offset( 0 );
		utility::convert_big_endian( &data[2], 3, &composition_time_offset );
		if( ( composition_time_offset & 0x800000 ) != 0 )
			composition_time_offset |= 0xffffffffff000000;
		args->SetPresentationTimestamp( packet.timestamp_ + composition_time_offset );

		const uint8 start_code[3] = { 0x00, 0x00, 0x01 };
		const auto length_size_minus_one = lengthSizeMinusOne_;

		auto itr = data.cbegin() + 5;
		std::basic_ostringstream<uint8> st;
		do
		{
			uint32 length( 0 );
			if( length_size_minus_one == 0x3 )
			{
				utility::convert_big_endian( &itr[0], 4, &length );
				itr += 4;
			}
			else if( length_size_minus_one == 0x1 )
			{
				utility::convert_big_endian( &itr[0], 2, &length );
				itr += 2;
			}
			else if( length_size_minus_one == 0x0 )
			{
				length = *itr++;
			}
			else
			{
				throw ref new Platform::FailureException();
			}

			st.write( start_code, 3 );
			st.write( &itr[0], length );
			itr += length;
		} while( itr < data.cend() );

		auto out = st.str();
		std::vector<uint8> buf( out.cbegin(), out.cend() );
		args->SetData( std::move( buf ) );

		VideoReceived( this, args );
		return;
	}

	args->SetPresentationTimestamp( packet.timestamp_ );

	// AVC sequence header (this is AVCDecoderConfigurationRecord)
	if( data[1] == 0x00 )
	{
		if( data.size() < 11 )
		{
			return;
		}

		if( !videoInfoEnabled_ )
		{
			const auto& dcr = *reinterpret_cast<avc_decoder_configuration_record*>( &data[5] );
			lengthSizeMinusOne_ = dcr.length_size_minus_one;
			videoInfo_->Format = VideoFormat::Avc;
			videoInfo_->ProfileIndication = static_cast<AvcProfileIndication>( dcr.avc_profile_indication );
			videoInfoEnabled_ = true;
			VideoStarted( this, ref new NetStreamVideoStartedEventArgs( !audioEnabled_, videoInfo_ ) );
		}

		args->Info = videoInfo_;

		const uint8 start_code[3] = { 0x00, 0x00, 0x01 };

		auto itr = data.cbegin() + 10;
		std::basic_ostringstream<uint8> st;

		const uint8 sps_count = *itr++ & 0x1f;
		for( auto i = 0u; i < sps_count; ++i )
		{
			uint16 sps_length;
			utility::convert_big_endian( &itr[0], 2, &sps_length );
			itr += 2;

			st.write( start_code, 3 );
			st.write( &itr[0], sps_length );
			itr += sps_length;
		}

		const uint8 pps_count = *itr++;
		for( auto i = 0u; i < pps_count; ++i )
		{
			uint16 pps_length;
			utility::convert_big_endian( &itr[0], 2, &pps_length );
			itr += 2;

			st.write( start_code, 3 );
			st.write( &itr[0], pps_length );
			itr += pps_length;
		}

		auto out = st.str();
		std::vector<uint8> buf( out.cbegin(), out.cend() );
		args->SetData( std::move( buf ) );
	}
	// AVC end of sequence (lower level NALU sequence ender is not required or supported)
	else if( data[1] == 0x02 )
	{
		std::vector<uint8> buf( 4, 0 );
		buf[2] = 0x01; // startCode
		buf[3] = 0 /* fixed-pattern(1b) forbidden_zero_bit */
			| 0x60 /* uint(2b) nal_ref_idc */
			| 10 /* uint(5b) nal_unit_type */;

		args->Info = videoInfo_;
		args->SetData( std::move( buf ) );
	}
	VideoReceived( this, args );
}