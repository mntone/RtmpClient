#include "pch.h"
#include "RtmpHelper.h"

using namespace Mntone::Rtmp;

Mntone::Data::Amf::AmfArray^ RtmpHelper::ParseAmf0( std::vector<uint8> data )
{
	using namespace Mntone::Data::Amf;

	auto buf = ref new Platform::Array<uint8>( static_cast<uint32>( 4 + data.size() ) );
	buf[0] = 0x80;
	memcpy( buf->Data + 1, data.data(), data.size() );
	buf[buf->Length - 3] = buf[buf->Length - 2] = 0; buf[buf->Length - 1] = 9;

	AmfArray^ amfArray;
	if( !AmfArray::TryParse( buf, AmfEncodingType::Amf0, &amfArray ) )
		amfArray = nullptr;
	return std::move( amfArray );
}

Mntone::Data::Amf::AmfArray^ RtmpHelper::ParseAmf3( std::vector<uint8> data )
{
	throw ref new Platform::NotImplementedException();
}

NetStatusCodeType RtmpHelper::ParseNetConnectionConnectCode( const std::wstring code )
{
	NetStatusCodeType nsc;

	const auto lastPhrase = code.substr( 22 /* NetConnection.Connect. */ );
	if( lastPhrase == L"Success" )
		nsc = NetStatusCodeType::NetConnection_Connect_Success;
	else if( lastPhrase == L"Closed" )
		nsc = NetStatusCodeType::NetConnection_Connect_Closed;
	else if( lastPhrase == L"Failed" )
		nsc = NetStatusCodeType::NetConnection_Connect_Failed;
	else if( lastPhrase == L"Rejected" )
		nsc = NetStatusCodeType::NetConnection_Connect_Rejected;
	else if( lastPhrase == L"InvalidApp" )
		nsc = NetStatusCodeType::NetConnection_Connect_InvalidApp;
	else if( lastPhrase == L"AppShutdown" )
		nsc = NetStatusCodeType::NetConnection_ConnectApp_Shutdown;
	else
		nsc = NetStatusCodeType::NetConnection_Connect_Other;

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
			nsc = NetStatusCodeType::NetStream_Failed;
		else
			nsc = NetStatusCodeType::NetStream_Other;
	}
	else
	{
		const auto secondPhrase = code.substr( 10, dotPos - 10 );
		if( secondPhrase == L"Play" )
		{
			const auto lastPhrase = code.substr( 15 /* NetStream.Play. */ );
			if( lastPhrase == L"Start" )
				nsc = NetStatusCodeType::NetStream_Play_Start;
			else if( lastPhrase == L"Stop" )
				nsc = NetStatusCodeType::NetStream_Play_Stop;
			else if( lastPhrase == L"Reset" )
				nsc = NetStatusCodeType::NetStream_Play_Reset;
			else if( lastPhrase == L"PublishNotify" )
				nsc = NetStatusCodeType::NetStream_Play_PublishNotify;
			else if( lastPhrase == L"UnpublishNotify" )
				nsc = NetStatusCodeType::NetStream_Play_UnpublishNotify;
			else if( lastPhrase == L"Transition" )
				nsc = NetStatusCodeType::NetStream_Play_Transition;
			else if( lastPhrase == L"Switch" )
				nsc = NetStatusCodeType::NetStream_Play_Switch;
			else if( lastPhrase == L"Complete" )
				nsc = NetStatusCodeType::NetStream_Play_Complete;
			else if( lastPhrase == L"TransitionComplete" )
				nsc = NetStatusCodeType::NetStream_Play_TransitionComplete;
			else if( lastPhrase == L"InsufficientBw" )
				nsc = NetStatusCodeType::NetStream_Play_InsufficientBandwidth;
			else if( lastPhrase == L"Failed" )
				nsc = NetStatusCodeType::NetStream_Play_Failed;
			else if( lastPhrase == L"StreamNotFound" )
				nsc = NetStatusCodeType::NetStream_Playstream_NotFound;
			else if( lastPhrase == L"FileStructureInvalid" )
				nsc = NetStatusCodeType::NetStream_Play_FileStructureInvalid;
			else if( lastPhrase == L"NoSupportedTrackFound" )
				nsc = NetStatusCodeType::NetStream_Play_NoSupportedTrackFound;
			else
				nsc = NetStatusCodeType::NetStream_Play_Other;
		}
		else if( secondPhrase == L"Pause" )
		{
			const auto lastPhrase = code.substr( 16 /* NetStream.Pause. */ );
			if( lastPhrase == L"Notify" )
				nsc = NetStatusCodeType::NetStream_Pause_Notify;
			else
				nsc = NetStatusCodeType::NetStream_Pause_Other;
		}
		else if( secondPhrase == L"Unpause" )
		{
			const auto lastPhrase = code.substr( 18 /* NetStream.Unpause. */ );
			if( lastPhrase == L"Notify" )
				nsc = NetStatusCodeType::NetStream_Unpause_Notify;
			else
				nsc = NetStatusCodeType::NetStream_Unpause_Other;
		}
		else if( secondPhrase == L"Seek" )
		{
			const auto lastPhrase = code.substr( 15 /* NetStream.Seek. */ );
			if( lastPhrase == L"Notify" )
				nsc = NetStatusCodeType::NetStream_Seek_Notify;
			else if( lastPhrase == L"Failed" )
				nsc = NetStatusCodeType::NetStream_Seek_Failed;
			else if( lastPhrase == L"InvalidTime" )
				nsc = NetStatusCodeType::NetStream_Seek_InvalidTime;
			else
				nsc = NetStatusCodeType::NetStream_Seek_Other;
		}
		else if( secondPhrase == L"Publish" )
		{
			const auto lastPhrase = code.substr( 18 /* NetStream.Publish. */ );
			if( lastPhrase == L"Start" )
				nsc = NetStatusCodeType::NetStream_Publish_Start;
			else if( lastPhrase == L"Idle" )
				nsc = NetStatusCodeType::NetStream_Publish_Idle;
			else if( lastPhrase == L"BadName" )
				nsc = NetStatusCodeType::NetStream_Publish_BadName;
			else
				nsc = NetStatusCodeType::NetStream_Publish_Other;
		}
		else if( secondPhrase == L"Unpublish" )
		{
			const auto lastPhrase = code.substr( 20 /* NetStream.Unpublish. */ );
			if( lastPhrase == L"Success" )
				nsc = NetStatusCodeType::NetStream_Unpublish_Success;
			else
				nsc = NetStatusCodeType::NetStream_Unpublish_Other;
		}
		else if( secondPhrase == L"Record" )
		{
			const auto lastPhrase = code.substr( 17 /* NetStream.Record. */ );
			if( lastPhrase == L"Start" )
				nsc = NetStatusCodeType::NetStream_Record_Start;
			else if( lastPhrase == L"Stop" )
				nsc = NetStatusCodeType::NetStream_Record_Stop;
			else if( lastPhrase == L"NoAccess" )
				nsc = NetStatusCodeType::NetStream_Record_NoAccess;
			else if( lastPhrase == L"Failed" )
				nsc = NetStatusCodeType::NetStream_Record_Failed;
			else if( lastPhrase == L"DiskQuotaExceeded" )
				nsc = NetStatusCodeType::NetStream_Record_DiskQuotaExceeded;
			else
				nsc = NetStatusCodeType::NetStream_Record_Other;
		}
		else if( secondPhrase == L"Buffer" )
		{
			const auto lastPhrase = code.substr( 17 /* NetStream.Buffer. */ );
			if( lastPhrase == L"Empty" )
				nsc = NetStatusCodeType::NetStream_Buffer_Empty;
			else if( lastPhrase == L"Full" )
				nsc = NetStatusCodeType::NetStream_Buffer_Full;
			else if( lastPhrase == L"Flush" )
				nsc = NetStatusCodeType::NetStream_Buffer_Flush;
			else
				nsc = NetStatusCodeType::NetStream_Buffer_Other;
		}
		else if( secondPhrase == L"MulticastStream" )
		{
			const auto lastPhrase = code.substr( 26 /* NetStream.MulticastStream. */ );
			if( lastPhrase == L"Reset" )
				nsc = NetStatusCodeType::NetStream_MulticastStream_Reset;
			else
				nsc = NetStatusCodeType::NetStream_MulticastStream_Other;
		}
		else
			nsc = NetStatusCodeType::NetStream_Other;
	}

	return nsc;
}