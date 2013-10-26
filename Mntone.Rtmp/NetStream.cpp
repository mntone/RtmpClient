#include "pch.h"
#include "sound_info.h"
#include "adts_header.h"
#include "video_type.h"
#include "VideoFormat.h"
#include "NetStream.h"
#include "NetConnection.h"
#include "RtmpHelper.h"

using namespace Mntone::Rtmp;

NetStream::NetStream( void ) :
	_streamId( 0 ),
	_audioInfoEnabled( false ),
	_audioInfo( ref new AudioInfo() ),
	_videoInfoEnabled( false ),
	_videoInfo( ref new VideoInfo() ),
	_length_size_minus_one( 0 )
{ }

NetStream::NetStream( NetConnection^ connection ) :
	NetStream()
{
	connection->AttachNetStream( this );
}

NetStream::~NetStream( void )
{
	using namespace Mntone::Data::Amf;

	auto cmd = ref new AmfArray();
	cmd->Append( AmfValue::CreateStringValue( "closeStream" ) );	// Command name
	cmd->Append( AmfValue::CreateDoubleValue( 0.0 ) );				// Transaction id
	cmd->Append( ref new AmfValue() );								// Command object: set to null type
	cmd->Append( AmfValue::CreateDoubleValue( _streamId ) );
	SendWithAction( cmd );

	_parent->UnattachNetStream( this );
}

void NetStream::Attach( NetConnection^ connection )
{
	connection->AttachNetStream( this );
}

void NetStream::__Attached( void )
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
	cmd->Append( AmfValue::CreateDoubleValue( 0.0 ) );		// Transaction id
	cmd->Append( ref new AmfValue() );						// Command object: set to null type
	cmd->Append( AmfValue::CreateStringValue( streamName ) );
	if( start != -2 )
	{
		cmd->Append( AmfValue::CreateDoubleValue( static_cast<float64>( start ) ) );
		if( duration != -1 )
			cmd->Append( AmfValue::CreateDoubleValue( static_cast<float64>( duration ) ) );
	}
	SendWithAction( cmd );
}

void NetStream::Pause( void ) { }
void NetStream::Resume( void ) { }
void NetStream::Seek( uint32 /*offset*/ ) { }

void NetStream::OnMessage( const rtmp_packet packet, std::vector<uint8> data )
{
	switch( packet.TypeId )
	{
	case type_id_type::tid_audio_message: OnAudioMessage( std::move( packet ), std::move( data ) ); break;
	case type_id_type::tid_video_message: OnVideoMessage( std::move( packet ), std::move( data ) ); break;
	case type_id_type::tid_data_message_amf0: OnDataMessageAmf0( std::move( packet ), std::move( data ) ); break;
	case type_id_type::tid_command_message_amf0: OnCommandMessageAmf0( std::move( packet ), std::move( data ) ); break;
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
			args->Info = _audioInfo;
			args->SetTimestamp( packet.Timestamp );
			args->SetData( std::move( data ), 2 );
			AudioReceived( this, args );
		}
		else if( data[1] == 0x00 )
		{
			auto& adts = *reinterpret_cast<adts_header*>( data.data() );
			_audioInfo->Format = AudioFormat::Aac;
			_audioInfo->SampleRate = adts.get_sampling_frequency_as_uint();
			_audioInfo->ChannelCount = adts.get_channel_configuration();
			_audioInfo->BitsPerSample = si.size == sound_size::ss_16bit ? 16 : 8;
			AudioStarted( this, ref new NetStreamAudioStartedEventArgs( _audioInfo ) );
		}
		return;
	}

	if( !_audioInfoEnabled )
	{
		switch( si.rate )
		{
		case sound_rate::sr_5_5khz: _audioInfo->SampleRate = 5513; break;
		case sound_rate::sr_11khz: _audioInfo->SampleRate = 11025; break;
		case sound_rate::sr_22khz: _audioInfo->SampleRate = 22050; break;
		case sound_rate::sr_44khz: _audioInfo->SampleRate = 44100; break;
		default: return;
		}

		switch( si.format )
		{
		case sound_format::sf_lpcm:	_audioInfo->Format = AudioFormat::Lpcm; break;
		case sound_format::sf_adpcm: _audioInfo->Format = AudioFormat::Adpcm; break;
		case sound_format::sf_mp3: _audioInfo->Format = AudioFormat::Mp3; break;
		case sound_format::sf_lpcm_little_endian: _audioInfo->Format = AudioFormat::LpcmLe; break;
		case sound_format::sf_nellymoser16khz_mono:
			_audioInfo->Format = AudioFormat::Nellymoser;
			_audioInfo->SampleRate = 16000;
			break;
		case sound_format::sf_nellymoser8khz_mono:
			_audioInfo->Format = AudioFormat::Nellymoser;
			_audioInfo->SampleRate = 8000;
			break;
		case sound_format::sf_nellymoser: _audioInfo->Format = AudioFormat::Nellymoser; break;
		case sound_format::sf_g711_alaw_logarithmic_pcm: _audioInfo->Format = AudioFormat::G711Alaw; break;
		case sound_format::sf_g711_mulaw_logarithmic_pcm: _audioInfo->Format = AudioFormat::G711Mulaw; break;
		case sound_format::sf_speex: _audioInfo->Format = AudioFormat::Speex; break;
		case sound_format::sf_mp38khz:
			_audioInfo->Format = AudioFormat::Mp3;
			_audioInfo->SampleRate = 8000;
			break;
		default: return;
		}

		_audioInfo->ChannelCount = si.type == sound_type::st_stereo ? 2 : 1;
		_audioInfo->BitsPerSample = si.size == sound_size::ss_16bit ? 16 : 8;
		_audioInfoEnabled = true;
		AudioStarted( this, ref new NetStreamAudioStartedEventArgs( _audioInfo ) );
	}

	auto args = ref new NetStreamAudioReceivedEventArgs( );
	args->Info = _audioInfo;
	args->SetTimestamp( packet.Timestamp );
	args->SetData( std::move( data ), 1 );
	AudioReceived( this, args );
}

void NetStream::OnVideoMessage( const rtmp_packet packet, std::vector<uint8> data )
{
	const auto& vt = static_cast<video_type>( ( data[0] >> 4 ) & 0x0f );
	const auto& vf = static_cast<VideoFormat>( data[0] & 0x0f );

	auto args = ref new NetStreamVideoReceivedEventArgs();
	args->IsKeyframe = vt == video_type::vt_keyframe;
	args->SetDecodeTimestamp( packet.Timestamp );

	if( vf == VideoFormat::Avc )
	{
		// Need to convert NAL file stream to byte stream
		AnalysisAvc( std::move( packet ), std::move( data ), args );
		VideoReceived( this, args );
		return;
	}

	if( !_videoInfoEnabled )
	{
		_videoInfo->Format = vf;
		_videoInfoEnabled = true;
	}

	args->Info = _videoInfo;
	args->SetPresentationTimestamp( packet.Timestamp );
	args->SetData( std::move( data ), 1 );
	VideoReceived( this, args );
}

void NetStream::AnalysisAvc( const rtmp_packet packet, std::vector<uint8> data, NetStreamVideoReceivedEventArgs^& args )
{
	// AVC NALU
	if( data[1] == 0x01 )
	{
		args->Info = _videoInfo;

		int64 compositionTimeOffset( 0 );
		ConvertBigEndian( data.data() + 2, &compositionTimeOffset, 3 );
		if( ( compositionTimeOffset & 0x800000 ) != 0 )
			compositionTimeOffset |= 0xffffffffff000000;
		args->SetPresentationTimestamp( packet.Timestamp + compositionTimeOffset );

		const uint8 startCode[3] = { 0x00, 0x00, 0x01 };
		auto p = data.data() + 5;
		const auto ep = data.data() + data.size();
		std::basic_stringstream<uint8> st;
		do
		{
			uint32 length( 0 );
			if( _length_size_minus_one == 0x03 )
			{
				ConvertBigEndian( p, &length, 4 );
				p += 4;
			}
			else if( _length_size_minus_one == 0x01 )
			{
				ConvertBigEndian( p, &length, 2 );
				p += 2;
			}
			else if( _length_size_minus_one == 0x00 )
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

	args->SetPresentationTimestamp( packet.Timestamp );

	// AVC sequence header (this is AVCDecoderConfigurationRecord)
	if( data[1] == 0x00 )
	{
		const auto& dcr = *reinterpret_cast<avc_decoder_configuration_record*>( data.data() + 5 );
		_length_size_minus_one = dcr.length_size_minus_one;
		_videoInfo->Format = VideoFormat::Avc;
		_videoInfoEnabled = true;
		VideoStarted( this, ref new NetStreamVideoStartedEventArgs( _videoInfo ) );

		args->Info = _videoInfo;

		const uint8 startCode[3] = { 0x00, 0x00, 0x01 };
		auto p = data.data() + 10;
		std::basic_stringstream<uint8> st;
		const uint8 spsCount = *( p++ ) & 0x1f;
		for( auto i = 0u; i < spsCount; ++i )
		{
			uint16 spsLength;
			ConvertBigEndian( p, &spsLength, 2 );
			p += 2;

			st.write( startCode, 3 );
			st.write( p, spsLength );

			p += spsLength;
		}
		const uint8 ppsCount = *( p++ );
		for( auto i = 0u; i < ppsCount; ++i )
		{
			uint16 ppsLength;
			ConvertBigEndian( p, &ppsLength, 2 );
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

		args->Info = _videoInfo;
		args->SetData( std::move( buf ) );
	}
}

void NetStream::OnDataMessageAmf0( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	OnDataMessage( std::move( RtmpHelper::ParseAmf0( std::move( data ) ) ) );
}

void NetStream::OnDataMessageAmf3( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	if( _parent->DefaultEncodingType == Mntone::Data::Amf::AmfEncodingType::Amf3 )
	{
	}
	//OnDataMessage( std::move( NetConnection::ParseAmf3( std::move( data ) ) ) );
	else
		OnDataMessage( std::move( RtmpHelper::ParseAmf0( std::move( data ) ) ) );
}

void NetStream::OnDataMessage( Mntone::Data::Amf::AmfArray^ amf )
{
}

void NetStream::OnCommandMessageAmf0( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	OnCommandMessage( std::move( RtmpHelper::ParseAmf0( std::move( data ) ) ) );
}

void NetStream::OnCommandMessageAmf3( const rtmp_packet /*packet*/, std::vector<uint8> data )
{
	if( _parent->DefaultEncodingType == Mntone::Data::Amf::AmfEncodingType::Amf3 )
	{ }
		//OnCommandMessage( std::move( NetConnection::ParseAmf3( std::move( data ) ) ) );
	else
		OnCommandMessage( std::move( RtmpHelper::ParseAmf0( std::move( data ) ) ) );
}

void NetStream::OnCommandMessage( Mntone::Data::Amf::AmfArray^ amf )
{
	const auto name = amf->GetStringAt( 0 );
	const auto information = amf->GetObjectAt( 3 );

	if( name != "onStatus" )
		return;

	const auto codePstr = information->GetNamedString( "code" );

	NetStatusType netStatus;
	{
		const std::wstring codeStr( codePstr->Data() );
		const auto dotPos = codeStr.find( L'.', 10 /* NetStream. */ );
		if( dotPos == std::wstring::npos )
		{
			const auto secondPhrase = codeStr.substr( 10 );
			if( secondPhrase == L"Failed" )
				netStatus = NetStatusType::NetStream_Failed;
			else
				netStatus = NetStatusType::NetStream_Other;
		}
		else
		{
			const auto secondPhrase = codeStr.substr( 10, dotPos - 10 );
			if( secondPhrase == L"Play" )
			{
				const auto lastPhrase = codeStr.substr( 15 /* NetStream.Play. */ );
				if( lastPhrase == L"Start" )
					netStatus = NetStatusType::NetStream_Play_Start;
				else if( lastPhrase == L"Stop" )
					netStatus = NetStatusType::NetStream_Play_Stop;
				else if( lastPhrase == L"Reset" )
					netStatus = NetStatusType::NetStream_Play_Reset;
				else if( lastPhrase == L"PublishNotify" )
					netStatus = NetStatusType::NetStream_Play_PublishNotify;
				else if( lastPhrase == L"UnpublishNotify" )
					netStatus = NetStatusType::NetStream_Play_UnpublishNotify;
				else if( lastPhrase == L"InsufficientBw" )
					netStatus = NetStatusType::NetStream_Play_InsufficientBw;
				else if( lastPhrase == L"Failed" )
					netStatus = NetStatusType::NetStream_Play_Failed;
				else if( lastPhrase == L"StreamNotFound" )
					netStatus = NetStatusType::NetStream_Play_StreamNotFound;
				else
					netStatus = NetStatusType::NetStream_Play_Other;
			}
			else if( secondPhrase == L"Pause" )
			{
				const auto lastPhrase = codeStr.substr( 16 /* NetStream.Pause. */ );
				if( lastPhrase == L"Notify" )
					netStatus = NetStatusType::NetStream_Pause_Notify;
				else
					netStatus = NetStatusType::NetStream_Pause_Other;
			}
			else if( secondPhrase == L"Unpause" )
			{
				const auto lastPhrase = codeStr.substr( 18 /* NetStream.Unpause. */ );
				if( lastPhrase == L"Notify" )
					netStatus = NetStatusType::NetStream_Unpause_Notify;
				else
					netStatus = NetStatusType::NetStream_Unpause_Other;
			}
			else if( secondPhrase == L"Seek" )
			{
				const auto lastPhrase = codeStr.substr( 15 /* NetStream.Seek. */ );
				if( lastPhrase == L"Notify" )
					netStatus = NetStatusType::NetStream_Seek_Notify;
				else if( lastPhrase == L"Failed" )
					netStatus = NetStatusType::NetStream_Seek_Failed;
				else if( lastPhrase == L"InvalidTime" )
					netStatus = NetStatusType::NetStream_Seek_InvalidTime;
				else
					netStatus = NetStatusType::NetStream_Seek_Other;
			}
			else if( secondPhrase == L"Publish" )
			{
				const auto lastPhrase = codeStr.substr( 18 /* NetStream.Publish. */ );
				if( lastPhrase == L"Start" )
					netStatus = NetStatusType::NetStream_Publish_Start;
				else if( lastPhrase == L"Idle" )
					netStatus = NetStatusType::NetStream_Publish_Idle;
				else if( lastPhrase == L"BadName" )
					netStatus = NetStatusType::NetStream_Publish_BadName;
				else
					netStatus = NetStatusType::NetStream_Publish_Other;
			}
			else if( secondPhrase == L"Unpublish" )
			{
				const auto lastPhrase = codeStr.substr( 20 /* NetStream.Unpublish. */ );
				if( lastPhrase == L"Success" )
					netStatus = NetStatusType::NetStream_Unpublish_Success;
				else
					netStatus = NetStatusType::NetStream_Unpublish_Other;
			}
			else if( secondPhrase == L"Record" )
			{
				const auto lastPhrase = codeStr.substr( 17 /* NetStream.Record. */ );
				if( lastPhrase == L"Start" )
					netStatus = NetStatusType::NetStream_Record_Start;
				else if( lastPhrase == L"Stop" )
					netStatus = NetStatusType::NetStream_Record_Stop;
				else if( lastPhrase == L"Failed" )
					netStatus = NetStatusType::NetStream_Record_Failed;
				else if( lastPhrase == L"NoAccess" )
					netStatus = NetStatusType::NetStream_Record_NoAccess;
				else
					netStatus = NetStatusType::NetStream_Record_Other;
			}
			else if( secondPhrase == L"Buffer" )
			{
				const auto lastPhrase = codeStr.substr( 17 /* NetStream.Buffer. */ );
				if( lastPhrase == L"Empty" )
					netStatus = NetStatusType::NetStream_Buffer_Empty;
				else if( lastPhrase == L"Full" )
					netStatus = NetStatusType::NetStream_Buffer_Full;
				else if( lastPhrase == L"Flush" )
					netStatus = NetStatusType::NetStream_Buffer_Flush;
				else
					netStatus = NetStatusType::NetStream_Buffer_Other;
			}
			else
				netStatus = NetStatusType::NetStream_Other;
		}

		StatusUpdated( this, ref new NetStatusUpdatedEventArgs( netStatus ) );
	}
}

void NetStream::SendWithAction( Mntone::Data::Amf::AmfArray^ amf )
{
	_parent->SendWithAction( _streamId, amf );
}