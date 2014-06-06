#include "pch.h"
#include "NetStream.h"
#include "NetConnection.h"
#include "RtmpHelper.h"
#include "Media/sound_info.h"
#include "Media/audio_specific_config.h"
#include "Media/video_type.h"
#include "Media/VideoFormat.h"
#include "Media/flv_tag.h"

using namespace Concurrency;
using namespace Windows::Foundation;
using namespace mntone::rtmp;
using namespace mntone::rtmp::media;
using namespace Mntone::Rtmp;
using namespace Mntone::Rtmp::Media;

NetStream::NetStream()
	: streamId_( 0 )
	, audioEnabled_( true ), audioInfoEnabled_( false ), audioInfo_( ref new AudioInfo() )
	, videoEnabled_( true ), videoInfoEnabled_( false ), videoInfo_( ref new VideoInfo() )
	, videoDataRate_( 0 ), videoHeight_( 0 ), videoWidth_( 0 )
	, lengthSizeMinusOne_( 0 )
{ }

NetStream::~NetStream()
{
	DetachedImpl();
}

IAsyncAction^ NetStream::AttachAsync( NetConnection^ connection )
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

void NetStream::DetachedImpl()
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
		parent_ = nullptr;
	}
}

IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName )
{
	return PlayAsync( streamName, -2 );
}

IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName, float64 start )
{
	return PlayAsync( streamName, start, -1 );
}

IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName, float64 start, float64 duration )
{
	return PlayAsync( streamName, start, duration, -1 );
}

IAsyncAction^ NetStream::PlayAsync( Platform::String^ streamName, float64 start, float64 duration, int16 reset )
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

IAsyncAction^ NetStream::PauseAsync( float64 position )
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

IAsyncAction^ NetStream::ResumeAsync( float64 position )
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

IAsyncAction^ NetStream::SeekAsync( float64 offset )
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

void NetStream::OnMessage( rtmp_header header, std::vector<uint8> data )
{
	switch( header.type_id )
	{
	case type_id_type::audio_message:
		OnAudioMessage( std::move( header ), std::move( data ) );
		break;

	case type_id_type::video_message:
		OnVideoMessage( std::move( header ), std::move( data ) );
		break;

	case type_id_type::data_message_amf3:
	case type_id_type::data_message_amf0:
		OnDataMessage( std::move( header ), std::move( data ) );
		break;

	case type_id_type::command_message_amf3:
	case type_id_type::command_message_amf0:
		OnCommandMessage( std::move( header ), std::move( data ) );
		break;

	case type_id_type::aggregate_message:
		OnAggregateMessage( std::move( header ), std::move( data ) );
		break;
	}
}

void NetStream::OnAudioMessage( rtmp_header header, std::vector<uint8> data )
{
	const auto& si = *reinterpret_cast<const sound_info*>( data.data() );

	if( si.format == sound_format::aac )
	{
		if( data.size() < 3 )
		{
			return;
		}

		if( data[1] == 0x01 )
		{
			auto args = ref new NetStreamAudioReceivedEventArgs();
			args->Info = audioInfo_;
			args->SetTimestamp( header.timestamp );
			args->SetData( std::move( data ), 2 );
			AudioReceived( this, args );
		}
		else if( data[1] == 0x00 && !audioInfoEnabled_ )
		{
			audio_specific_config asc( data.data() + 2, data.size() - 2 );
			audioInfo_->Format = AudioFormat::Aac;
			audioInfo_->SampleRate = asc.sampling_frequency();
			audioInfo_->ChannelCount = asc.channel_configuration();
			audioInfo_->BitsPerSample = si.size == sound_size::s16bit ? 16 : 8;
			audioInfoEnabled_ = true;
			AudioStarted( this, ref new NetStreamAudioStartedEventArgs( !videoEnabled_, audioInfo_ ) );
		}
		return;
	}

	if( !audioInfoEnabled_ )
	{
		audioInfo_->SetInfo( si );
		audioInfoEnabled_ = true;
		AudioStarted( this, ref new NetStreamAudioStartedEventArgs( !videoEnabled_, audioInfo_ ) );
	}

	auto args = ref new NetStreamAudioReceivedEventArgs();
	args->Info = audioInfo_;
	args->SetTimestamp( header.timestamp );
	args->SetData( std::move( data ), 1 );
	AudioReceived( this, args );
}

void NetStream::OnVideoMessage( rtmp_header header, std::vector<uint8> data )
{
	const auto& vt = static_cast<video_type>( ( data[0] >> 4 ) & 0x0f );
	const auto& vf = static_cast<VideoFormat>( data[0] & 0x0f );

	auto args = ref new NetStreamVideoReceivedEventArgs();
	args->IsKeyframe = vt == video_type::keyframe;
	args->SetDecodeTimestamp( header.timestamp );

	if( vf == VideoFormat::Avc )
	{
		// Need to convert NAL file stream to byte stream
		AnalysisAvc( std::move( header ), std::move( data ), args );
		return;
	}

	if( !videoInfoEnabled_ )
	{
		videoInfo_->Format = vf;
		videoInfo_->Bitrate = videoDataRate_;
		videoInfo_->Height = videoHeight_;
		videoInfo_->Width = videoWidth_;
		videoInfoEnabled_ = true;
		VideoStarted( this, ref new NetStreamVideoStartedEventArgs( !audioEnabled_, videoInfo_ ) );
	}

	args->Info = videoInfo_;
	args->SetPresentationTimestamp( header.timestamp );
	args->SetData( std::move( data ), 1 );
	VideoReceived( this, args );
}

void NetStream::OnDataMessage( rtmp_header /*header*/, std::vector<uint8> data )
{
	const auto& amf = RtmpHelper::ParseAmf( std::move( data ) );
	const auto& name = amf->GetStringAt( 0 );
	if( name != "onMetaData" )
	{
		return;
	}

	const auto& object = amf->GetObjectAt( 1 );

	if( object->HasKey( "videocodecid" ) )
	{
		videoEnabled_ = true;
		if( object->HasKey( "videodatarate" ) )
		{
			videoDataRate_ = static_cast<uint16>( object->GetNamedNumber( "videodatarate" ) );
		}
		if( object->HasKey( "height" ) )
		{
			videoHeight_ = static_cast<uint16>( object->GetNamedNumber( "height" ) );
		}
		if( object->HasKey( "width" ) )
		{
			videoWidth_ = static_cast<uint16>( object->GetNamedNumber( "width" ) );
		}
	}
	else
	{
		videoEnabled_ = false;
	}

	if( object->HasKey( "audiocodecid" ) )
	{
		audioEnabled_ = true;
	}
	else
	{
		audioEnabled_ = false;
	}
}

void NetStream::OnCommandMessage( rtmp_header /*header*/, std::vector<uint8> data )
{
	const auto& amf = RtmpHelper::ParseAmf( std::move( data ) );
	const auto& name = amf->GetStringAt( 0 );
	if( name != "onStatus" )
	{
		return;
	}

	const auto& information = amf->GetObjectAt( 3 );
	const auto& code = information->GetNamedString( "code" );
	const auto& nsc = RtmpHelper::ParseNetStreamCode( code->Data() );
	StatusUpdated( this, ref new NetStatusUpdatedEventArgs( nsc ) );
}

void NetStream::OnAggregateMessage( rtmp_header header, std::vector<uint8> data )
{
	if( data.size() < 11 )
	{
		return;
	}

	auto itr = data.cbegin();
	do
	{
		const auto& tag = *reinterpret_cast<const flv_tag*>( &itr[0] );
		itr += 11;

		auto clone_header = header;
		clone_header.timestamp = tag.timestamp();
		clone_header.type_id = static_cast<type_id_type>( tag.tag_type() );

		auto end_of_sequence = itr + tag.data_size();

		std::vector<uint8> subset_data( itr, end_of_sequence );
		switch( tag.tag_type() )
		{
		case flv_tag_type::audio:
			OnAudioMessage( std::move( clone_header ), std::move( subset_data ) );
			break;

		case flv_tag_type::video:
			OnVideoMessage( std::move( clone_header ), std::move( subset_data ) );
			break;

		case flv_tag_type::script_data:
			OnDataMessage( std::move( clone_header ), std::move( subset_data ) );
			break;
		}
		itr = end_of_sequence + 4;
	} while( itr < data.cend() );
}

task<void> NetStream::SendActionAsync( Mntone::Data::Amf::AmfArray^ amf )
{
	if( parent_ != nullptr )
	{
		return parent_->SendActionAsync( streamId_, amf );
	}

	return create_task( [] { } );
}