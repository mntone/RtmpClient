#include "pch.h"
#include "RtmpHelper.h"

using namespace Mntone::Rtmp;

Mntone::Data::Amf::AmfArray^ RtmpHelper::ParseAmf( std::vector<uint8> data )
{
	using namespace Mntone::Data::Amf;

	auto buf = ref new Platform::Array<uint8>( static_cast<uint32>( 4 + data.size() ) );
	buf[0] = 0x80;
	memcpy( buf->begin() + 1, data.data(), data.size() );
	buf[buf->Length - 3] = buf[buf->Length - 2] = 0; buf[buf->Length - 1] = 9;

	AmfArray^ ary;
	if( !AmfArray::TryParse( buf, AmfEncodingType::Amf0, &ary ) )
	{
		ary = nullptr;
	}
	return ary;
}

NetStatusCodeType RtmpHelper::ParseNetConnectionConnectCode( const std::wstring code )
{
	NetStatusCodeType nsc;

	const auto lastPhrase = code.substr( 22 /* NetConnection.Connect. */ );
	if( lastPhrase == L"Success" )
		nsc = NetStatusCodeType::NetConnectionConnectSuccess;
	else if( lastPhrase == L"Closed" )
		nsc = NetStatusCodeType::NetConnectionConnectClosed;
	else if( lastPhrase == L"Failed" )
		nsc = NetStatusCodeType::NetConnectionConnectFailed;
	else if( lastPhrase == L"Rejected" )
		nsc = NetStatusCodeType::NetConnectionConnectRejected;
	else if( lastPhrase == L"InvalidApp" )
		nsc = NetStatusCodeType::NetConnectionConnectInvalidApp;
	else if( lastPhrase == L"AppShutdown" )
		nsc = NetStatusCodeType::NetConnectionConnectAppShutdown;
	else
		nsc = NetStatusCodeType::NetConnectionConnectOther;

	return nsc;
}

NetStatusCodeType RtmpHelper::ParseNetStreamCode( const std::wstring code )
{
	NetStatusCodeType nsc;

	const auto dotPos = code.find( L'.', 10 /* NetStream. */ );
	if( dotPos == std::wstring::npos )
	{
		const auto secondPhrase = code.substr( 10 );
		if( secondPhrase == L"Failed" )
			nsc = NetStatusCodeType::NetStreamFailed;
		else
			nsc = NetStatusCodeType::NetStreamOther;
	}
	else
	{
		const auto secondPhrase = code.substr( 10, dotPos - 10 );
		if( secondPhrase == L"Play" )
		{
			const auto lastPhrase = code.substr( 15 /* NetStream.Play. */ );
			if( lastPhrase == L"Start" )
				nsc = NetStatusCodeType::NetStreamPlayStart;
			else if( lastPhrase == L"Stop" )
				nsc = NetStatusCodeType::NetStreamPlayStop;
			else if( lastPhrase == L"Reset" )
				nsc = NetStatusCodeType::NetStreamPlayReset;
			else if( lastPhrase == L"PublishNotify" )
				nsc = NetStatusCodeType::NetStreamPlayPublishNotify;
			else if( lastPhrase == L"UnpublishNotify" )
				nsc = NetStatusCodeType::NetStreamPlayUnpublishNotify;
			else if( lastPhrase == L"Transition" )
				nsc = NetStatusCodeType::NetStreamPlayTransition;
			else if( lastPhrase == L"Switch" )
				nsc = NetStatusCodeType::NetStreamPlaySwitch;
			else if( lastPhrase == L"Complete" )
				nsc = NetStatusCodeType::NetStreamPlayComplete;
			else if( lastPhrase == L"TransitionComplete" )
				nsc = NetStatusCodeType::NetStreamPlayTransitionComplete;
			else if( lastPhrase == L"InsufficientBw" )
				nsc = NetStatusCodeType::NetStreamPlayInsufficientBandwidth;
			else if( lastPhrase == L"Failed" )
				nsc = NetStatusCodeType::NetStreamPlayFailed;
			else if( lastPhrase == L"StreamNotFound" )
				nsc = NetStatusCodeType::NetStreamPlayStreamNotFound;
			else if( lastPhrase == L"FileStructureInvalid" )
				nsc = NetStatusCodeType::NetStreamPlayFileStructureInvalid;
			else if( lastPhrase == L"NoSupportedTrackFound" )
				nsc = NetStatusCodeType::NetStreamPlayNoSupportedTrackFound;
			else
				nsc = NetStatusCodeType::NetStreamPlayOther;
		}
		else if( secondPhrase == L"Pause" )
		{
			const auto lastPhrase = code.substr( 16 /* NetStream.Pause. */ );
			if( lastPhrase == L"Notify" )
				nsc = NetStatusCodeType::NetStreamPauseNotify;
			else
				nsc = NetStatusCodeType::NetStreamPauseOther;
		}
		else if( secondPhrase == L"Unpause" )
		{
			const auto lastPhrase = code.substr( 18 /* NetStream.Unpause. */ );
			if( lastPhrase == L"Notify" )
				nsc = NetStatusCodeType::NetStreamUnpauseNotify;
			else
				nsc = NetStatusCodeType::NetStreamUnpauseOther;
		}
		else if( secondPhrase == L"Seek" )
		{
			const auto lastPhrase = code.substr( 15 /* NetStream.Seek. */ );
			if( lastPhrase == L"Notify" )
				nsc = NetStatusCodeType::NetStreamSeekNotify;
			else if( lastPhrase == L"Failed" )
				nsc = NetStatusCodeType::NetStreamSeekFailed;
			else if( lastPhrase == L"InvalidTime" )
				nsc = NetStatusCodeType::NetStreamSeekInvalidTime;
			else
				nsc = NetStatusCodeType::NetStreamSeekOther;
		}
		else if( secondPhrase == L"Publish" )
		{
			const auto lastPhrase = code.substr( 18 /* NetStream.Publish. */ );
			if( lastPhrase == L"Start" )
				nsc = NetStatusCodeType::NetStreamPublishStart;
			else if( lastPhrase == L"Idle" )
				nsc = NetStatusCodeType::NetStreamPublishIdle;
			else if( lastPhrase == L"BadName" )
				nsc = NetStatusCodeType::NetStreamPublishBadName;
			else
				nsc = NetStatusCodeType::NetStreamPublishOther;
		}
		else if( secondPhrase == L"Unpublish" )
		{
			const auto lastPhrase = code.substr( 20 /* NetStream.Unpublish. */ );
			if( lastPhrase == L"Success" )
				nsc = NetStatusCodeType::NetStreamUnpublishSuccess;
			else
				nsc = NetStatusCodeType::NetStreamUnpublishOther;
		}
		else if( secondPhrase == L"Record" )
		{
			const auto lastPhrase = code.substr( 17 /* NetStream.Record. */ );
			if( lastPhrase == L"Start" )
				nsc = NetStatusCodeType::NetStreamRecordStart;
			else if( lastPhrase == L"Stop" )
				nsc = NetStatusCodeType::NetStreamRecordStop;
			else if( lastPhrase == L"NoAccess" )
				nsc = NetStatusCodeType::NetStreamRecordNoAccess;
			else if( lastPhrase == L"Failed" )
				nsc = NetStatusCodeType::NetStreamRecordFailed;
			else if( lastPhrase == L"DiskQuotaExceeded" )
				nsc = NetStatusCodeType::NetStreamRecordDiskQuotaExceeded;
			else
				nsc = NetStatusCodeType::NetStreamRecordOther;
		}
		else if( secondPhrase == L"Buffer" )
		{
			const auto lastPhrase = code.substr( 17 /* NetStream.Buffer. */ );
			if( lastPhrase == L"Empty" )
				nsc = NetStatusCodeType::NetStreamBufferEmpty;
			else if( lastPhrase == L"Full" )
				nsc = NetStatusCodeType::NetStreamBufferFull;
			else if( lastPhrase == L"Flush" )
				nsc = NetStatusCodeType::NetStreamBufferFlush;
			else
				nsc = NetStatusCodeType::NetStreamBufferOther;
		}
		else if( secondPhrase == L"MulticastStream" )
		{
			const auto lastPhrase = code.substr( 26 /* NetStream.MulticastStream. */ );
			if( lastPhrase == L"Reset" )
				nsc = NetStatusCodeType::NetStreamMulticastStreamReset;
			else
				nsc = NetStatusCodeType::NetStreamMulticastStreamOther;
		}
		else
		{
			nsc = NetStatusCodeType::NetStreamOther;
		}
	}

	return nsc;
}

uint32 RtmpHelper::ReadUint24( Windows::Storage::Streams::DataReader^ reader )
{
	auto buf = ref new Platform::Array<uint8>( 3 );
	reader->ReadBytes( buf );

	uint32 ret( 0 );
	if( reader->ByteOrder == Windows::Storage::Streams::ByteOrder::BigEndian )
	{
		mntone::rtmp::utility::convert_big_endian( buf->Data, buf->Length, &ret );
	}
	else
	{
		mntone::rtmp::utility::convert_little_endian( buf->Data, buf->Length, &ret );
	}
	return ret;
}