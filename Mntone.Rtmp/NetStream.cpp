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
	using namespace Mntone::Data::Amf;

	auto cmd = ref new AmfArray();
	cmd->Append( AmfValue::CreateStringValue( "closeStream" ) );	// Command name
	cmd->Append( AmfValue::CreateNumberValue( 0.0 ) );				// Transaction id
	cmd->Append( ref new AmfValue() );								// Command object: set to null type
	cmd->Append( AmfValue::CreateNumberValue( streamId_ ) );
	SendActionAsync( cmd );

	parent_->UnattachNetStream( this );
}

Windows::Foundation::IAsyncAction^ NetStream::AttachAsync( NetConnection^ connection )
{
	return create_async( [=]
	{
		return connection->AttachNetStream( this );
	} );
}

void NetStream::AttachedImpl()
{
	Attached( this, ref new NetStreamAttachedEventArgs() );
}

void NetStream::Play( Platform::String^ streamName )
{
	Play( streamName, -2 );
}

void NetStream::Play( Platform::String^ streamName, int32 start )
{
	Play( streamName, start, -1 );
}

void NetStream::Play( Platform::String^ streamName, int32 start, int32 duration )
{
	using namespace Mntone::Data::Amf;

	auto cmd = ref new AmfArray();
	cmd->Append( AmfValue::CreateStringValue( "play" ) );	// Command name
	cmd->Append( AmfValue::CreateNumberValue( 0.0 ) );		// Transaction id
	cmd->Append( ref new AmfValue() );						// Command object: set to null type
	cmd->Append( AmfValue::CreateStringValue( streamName ) );
	if( start != -2 )
	{
		cmd->Append( AmfValue::CreateNumberValue( static_cast<float64>( start ) ) );
		if( duration != -1 )
			cmd->Append( AmfValue::CreateNumberValue( static_cast<float64>( duration ) ) );
	}
	SendActionAsync( cmd );
}

void NetStream::Pause() { }
void NetStream::Resume() { }
void NetStream::Seek( uint32 /*offset*/ ) { }

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
			auto& adts = *reinterpret_cast<adts_header*>( data.data() );
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
		VideoReceived( this, args );
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

		int64 compositionTimeOffset( 0 );
		utility::convert_big_endian( data.data() + 2, 3, &compositionTimeOffset );
		if( ( compositionTimeOffset & 0x800000 ) != 0 )
			compositionTimeOffset |= 0xffffffffff000000;
		args->SetPresentationTimestamp( packet.timestamp_ + compositionTimeOffset );

		const uint8 startCode[3] = { 0x00, 0x00, 0x01 };
		auto p = data.data() + 5;
		const auto& ep = data.data() + data.size();
		std::basic_ostringstream<uint8> st;
		do
		{
			uint32 length( 0 );
			if( lengthSizeMinusOne_ == 0x03 )
			{
				utility::convert_big_endian( p, 4, &length );
				p += 4;
			}
			else if( lengthSizeMinusOne_ == 0x01 )
			{
				utility::convert_big_endian( p, 2, &length );
				p += 2;
			}
			else if( lengthSizeMinusOne_ == 0x00 )
				length = *( p++ );
			else
				throw ref new Platform::FailureException();

			st.write( startCode, 3 );
			st.write( p, length );

			p += length;
		} while( p < ep );

		auto out = st.str();
		std::vector<uint8> buf( out.size() );
		memcpy( buf.data(), out.c_str(), out.size() );
		args->SetData( std::move( buf ) );
		return;
	}

	args->SetPresentationTimestamp( packet.timestamp_ );

	// AVC sequence header (this is AVCDecoderConfigurationRecord)
	if( data[1] == 0x00 )
	{
		const auto& dcr = *reinterpret_cast<avc_decoder_configuration_record*>( data.data() + 5 );
		lengthSizeMinusOne_ = dcr.length_size_minus_one;
		videoInfo_->Format = VideoFormat::Avc;
		videoInfoEnabled_ = true;
		VideoStarted( this, ref new NetStreamVideoStartedEventArgs( videoInfo_ ) );

		args->Info = videoInfo_;

		const uint8 startCode[3] = { 0x00, 0x00, 0x01 };
		auto p = data.data() + 10;
		std::basic_ostringstream<uint8> st;
		const uint8 spsCount = *( p++ ) & 0x1f;
		for( auto i = 0u; i < spsCount; ++i )
		{
			uint16 spsLength;
			utility::convert_big_endian( p, 2, &spsLength );
			p += 2;

			st.write( startCode, 3 );
			st.write( p, spsLength );

			p += spsLength;
		}
		const uint8 ppsCount = *( p++ );
		for( auto i = 0u; i < ppsCount; ++i )
		{
			uint16 ppsLength;
			utility::convert_big_endian( p, 2, &ppsLength );
			p += 2;

			st.write( startCode, 3 );
			st.write( p, ppsLength );

			p += ppsLength;
		}

		auto out = st.str();
		std::vector<uint8> buf( out.size() );
		memcpy( buf.data(), out.c_str(), out.size() );
		args->SetData( std::move( buf ) );
	}
	// AVC end of sequence (lower level NALU sequence ender is not required or supported)
	else if( data[1] == 0x02 )
	{
		std::vector<uint8> buf( 4, 0 );
		buf[2] = 0x01; // startCode
		buf[3] = 0 /* fixed-pattern(1b) forbidden_zero_bit */ | 0x60 /* uint(2b) nal_ref_idc */ | 10 /* uint(5b) nal_unit_type */;

		args->Info = videoInfo_;
		args->SetData( std::move( buf ) );
	}
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
	return parent_->SendActionAsync( streamId_, amf );
}