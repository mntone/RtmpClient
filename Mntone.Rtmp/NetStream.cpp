#include "pch.h"
#include "sound_info.h"
#include "adts_header.h"
#include "video_type.h"
#include "VideoFormat.h"
#include "NetStream.h"
#include "NetConnection.h"
#include "RtmpHelper.h"

using namespace Concurrency;
using namespace mntone::rtmp;
using namespace Mntone::Rtmp;
namespace WF = Windows::Foundation;

NetStream::NetStream()
	: streamId_( 0 )
	, audioInfoEnabled_( false )
	, audioInfo_( ref new AudioInfo() )
	, videoInfoEnabled_( false )
	, videoInfo_( ref new VideoInfo() )
	, lengthSizeMinusOne_( 0 )
{ }

NetStream::~NetStream()
{
	if( parent_ != nullptr )
	{
		using namespace Mntone::Data::Amf;

		auto cmd = ref new AmfArray();
		cmd->Append( AmfValue::CreateStringValue( "closeStream" ) );	// Command name
		cmd->Append( AmfValue::CreateNumberValue( 0.0 ) );				// Transaction id
		cmd->Append( ref new AmfValue() );								// Command object: set to null type
		cmd->Append( AmfValue::CreateNumberValue( streamId_ ) );
		SendActionAsync( cmd );

		parent_->UnattachNetStream( this );
	}
}

WF::IAsyncAction^ NetStream::AttachAsync( NetConnection^ connection )
{
	return create_async( [=]
	{
		return connection->AttachNetStreamAsync( this );
	} );
}

void NetStream::AttachedImpl()
{
	Attached( this, ref new NetStreamAttachedEventArgs() );
}

void NetStream::UnattachedImpl()
{
	parent_ = nullptr;
}

WF::IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName )
{
	return PlayAsync( streamName, -2 );
}

WF::IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName, float64 start )
{
	return PlayAsync( streamName, start, -1 );
}

WF::IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName, float64 start, float64 duration )
{
	return PlayAsync( streamName, start, duration, -1 );
}

WF::IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName, float64 start, float64 duration, int16 reset )
{
	return create_async( [=]
	{
		using namespace Mntone::Data::Amf;

		auto cmd = ref new AmfArray();
		cmd->Append( AmfValue::CreateStringValue( "play" ) );	// Command name
		cmd->Append( AmfValue::CreateNumberValue( 0.0 ) );		// Transaction id
		cmd->Append( ref new AmfValue() );						// Command object: set to null type
		cmd->Append( AmfValue::CreateStringValue( streamName ) );
		if( start != -2.0 )
		{
			cmd->Append( AmfValue::CreateNumberValue( start ) );
			if( duration != -1.0 )
			{
				cmd->Append( AmfValue::CreateNumberValue( duration ) );
				if( reset != -1 )
				{
					cmd->Append( AmfValue::CreateNumberValue( static_cast<float64>( reset ) ) );
				}
			}
		}
		return SendActionAsync( cmd );
	} );
}

WF::IAsyncAction^ NetStream::PauseAsync( float64 position )
{
	return create_async( [=]
	{
		using namespace Mntone::Data::Amf;

		auto cmd = ref new AmfArray();
		cmd->Append( AmfValue::CreateStringValue( "pause" ) );	// Command name
		cmd->Append( AmfValue::CreateNumberValue( 0.0 ) );		// Transaction id
		cmd->Append( ref new AmfValue() );						// Command object: set to null type
		cmd->Append( AmfValue::CreateBooleanValue( true ) );
		cmd->Append( AmfValue::CreateNumberValue( position ) );
		return SendActionAsync( cmd );
	} );
}

WF::IAsyncAction^ NetStream::ResumeAsync( float64 position ) 
{
	return create_async( [=]
	{
		using namespace Mntone::Data::Amf;

		auto cmd = ref new AmfArray();
		cmd->Append( AmfValue::CreateStringValue( "pause" ) );	// Command name
		cmd->Append( AmfValue::CreateNumberValue( 0.0 ) );		// Transaction id
		cmd->Append( ref new AmfValue() );						// Command object: set to null type
		cmd->Append( AmfValue::CreateBooleanValue( false ) );
		cmd->Append( AmfValue::CreateNumberValue( position ) );
		return SendActionAsync( cmd );
	} );
}

WF::IAsyncAction^ NetStream::SeekAsync( float64 offset )
{
	return create_async( [=]
	{
		using namespace Mntone::Data::Amf;

		auto cmd = ref new AmfArray();
		cmd->Append( AmfValue::CreateStringValue( "seek" ) );	// Command name
		cmd->Append( AmfValue::CreateNumberValue( 0.0 ) );		// Transaction id
		cmd->Append( ref new AmfValue() );						// Command object: set to null type
		cmd->Append( AmfValue::CreateNumberValue( offset ) );
		return SendActionAsync( cmd );
	} );
}

void NetStream::OnMessage( const rtmp_packet packet, std::vector<uint8> data )
{
	switch( packet.type_id_ )
	{
	case type_id_type::tid_audio_message:
		OnAudioMessage( std::move( packet ), std::move( data ) );
		break;
	case type_id_type::tid_video_message:
		OnVideoMessage( std::move( packet ), std::move( data ) );
		break;
	case type_id_type::tid_data_message_amf3:
	case type_id_type::tid_data_message_amf0:
		OnDataMessage( std::move( packet ), std::move( data ) );
		break;
	case type_id_type::tid_command_message_amf3:
	case type_id_type::tid_command_message_amf0:
		OnCommandMessage( std::move( packet ), std::move( data ) );
		break;
	}
}

void NetStream::OnAudioMessage( const rtmp_packet packet, std::vector<uint8> data )
{
	const auto& si = *reinterpret_cast<sound_info*>( data.data() );

	if( si.format == sound_format::sf_aac )
	{
		if( data[1] == 0x01 )
		{
			auto args = ref new NetStreamAudioReceivedEventArgs();
			args->Info = audioInfo_;
			args->SetTimestamp( packet.timestamp_ );
			args->SetData( std::move( data ), 2 );
			AudioReceived( this, args );
		}
		else if( data[1] == 0x00 )
		{
			const auto& adts = *reinterpret_cast<adts_header*>( data.data() );
			audioInfo_->Format = AudioFormat::Aac;
			audioInfo_->SampleRate = adts.sampling_frequency_as_uint();
			audioInfo_->ChannelCount = adts.channel_configuration();
			audioInfo_->BitsPerSample = si.size == sound_size::ss_16bit ? 16 : 8;
			AudioStarted( this, ref new NetStreamAudioStartedEventArgs( audioInfo_ ) );
		}
		return;
	}

	if( !audioInfoEnabled_ )
	{
		audioInfo_->SetInfo( si );
		audioInfoEnabled_ = true;
		AudioStarted( this, ref new NetStreamAudioStartedEventArgs( audioInfo_ ) );
	}

	auto args = ref new NetStreamAudioReceivedEventArgs();
	args->Info = audioInfo_;
	args->SetTimestamp( packet.timestamp_ );
	args->SetData( std::move( data ), 1 );
	AudioReceived( this, args );
}

void NetStream::OnVideoMessage( const rtmp_packet packet, std::vector<uint8> data )
{
	const auto& vt = static_cast<video_type>( ( data[0] >> 4 ) & 0x0f );
	const auto& vf = static_cast<VideoFormat>( data[0] & 0x0f );

	auto args = ref new NetStreamVideoReceivedEventArgs();
	args->IsKeyframe = vt == video_type::vt_keyframe;
	args->SetDecodeTimestamp( packet.timestamp_ );

	if( vf == VideoFormat::Avc )
	{
		// Need to convert NAL file stream to byte stream
		AnalysisAvc( std::move( packet ), std::move( data ), args );
		return;
	}

	if( !videoInfoEnabled_ )
	{
		videoInfo_->Format = vf;
		videoInfoEnabled_ = true;
	}

	args->Info = videoInfo_;
	args->SetPresentationTimestamp( packet.timestamp_ );
	args->SetData( std::move( data ), 1 );
	VideoReceived( this, args );
}

void NetStream::AnalysisAvc( const rtmp_packet packet, std::vector<uint8> data, NetStreamVideoReceivedEventArgs^& args )
{
	// AVC NALU
	if( data[1] == 0x01 )
	{
		args->Info = videoInfo_;

		int64 composition_time_offset( 0 );
		utility::convert_big_endian( data.data() + 2, 3, &composition_time_offset );
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
				length = *itr++;
			else
				throw ref new Platform::FailureException();

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
		const auto& dcr = *reinterpret_cast<avc_decoder_configuration_record*>( &data[5] );
		lengthSizeMinusOne_ = dcr.length_size_minus_one;
		videoInfo_->Format = VideoFormat::Avc;
		videoInfoEnabled_ = true;
		VideoStarted( this, ref new NetStreamVideoStartedEventArgs( videoInfo_ ) );

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

void NetStream::OnDataMessage( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	const auto& amf = RtmpHelper::ParseAmf( std::move( data ) );
}

void NetStream::OnCommandMessage( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	const auto& amf = RtmpHelper::ParseAmf( std::move( data ) );
	const auto& name = amf->GetStringAt( 0 );
	const auto& information = amf->GetObjectAt( 3 );

	if( name != "onStatus" )
		return;

	const auto& code = information->GetNamedString( "code" );
	const auto& nsc = RtmpHelper::ParseNetStreamCode( code->Data() );
	StatusUpdated( this, ref new NetStatusUpdatedEventArgs( nsc ) );
}

task<void> NetStream::SendActionAsync( Mntone::Data::Amf::AmfArray^ amf )
{
	if( parent_ != nullptr )
		return parent_->SendActionAsync( streamId_, amf );

	return create_task( [] { } );
}