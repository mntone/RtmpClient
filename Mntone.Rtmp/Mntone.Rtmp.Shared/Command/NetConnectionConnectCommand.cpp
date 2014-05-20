#include "pch.h"
#include "NetConnectionConnectCommand.h"

using namespace Mntone::Rtmp::Command;

NetConnectionConnectCommand::NetConnectionConnectCommand( Platform::String^ app )
	: App_( app )
	, FlashVersion_( "WIN 9,0,262,0" )
	, SwfUrl_( "http://localhost/dummy.swf" )
	, Fpad_( false )
	, AudioCodecs_( SupportSoundType::Mp3 | SupportSoundType::Aac )
	, VideoCodecs_( SupportVideoType::Sorenson | SupportVideoType::H264 )
	, VideoFunction_( SupportVideoFunctionType::Seek )
	, PageUrl_( "http://localhost/dummy.html" )
	, ObjectEncoding_( Mntone::Data::Amf::AmfEncodingType::Amf0 )
{ }

Mntone::Data::Amf::AmfArray^ NetConnectionConnectCommand::Commandify()
{
	using namespace Mntone::Data::Amf;

	auto ary = ref new AmfArray();
	ary->Append( AmfValue::CreateStringValue( "connect" ) );	// Command name
	ary->Append( AmfValue::CreateNumberValue( 1.0 ) );			// Transaction id: always set to 1.

	auto obj = ref new AmfObject();
	obj->Insert( "app", AmfValue::CreateStringValue( App_ ) );
	obj->Insert( "flashVer", AmfValue::CreateStringValue( FlashVersion_ ) );
	obj->Insert( "swfUrl", AmfValue::CreateStringValue( SwfUrl_ ) );
	obj->Insert( "tcUrl", AmfValue::CreateStringValue( TcUrl_ ) );
	obj->Insert( "fpad", AmfValue::CreateBooleanValue( Fpad_ ) );
	obj->Insert( "audioCodecs", AmfValue::CreateNumberValue( static_cast<float64>( AudioCodecs_ ) ) );
	obj->Insert( "videoCodecs", AmfValue::CreateNumberValue( static_cast<float64>( VideoCodecs_ ) ) );
	obj->Insert( "videoFunction", AmfValue::CreateNumberValue( static_cast<float64>( VideoFunction_ ) ) );
	obj->Insert( "pageUrl", AmfValue::CreateStringValue( PageUrl_ ) );
	obj->Insert( "objectEncoding", AmfValue::CreateNumberValue( static_cast<float64>( ObjectEncoding_ ) ) );
	ary->Append( obj );

	ary->Append( OptionalUserArguments_ != nullptr ? OptionalUserArguments_ : ref new AmfValue() );

	return ary;
}

Platform::String^ NetConnectionConnectCommand::ToString()
{
	return "connect (" + App_ + ")";
}