#include "pch.h"
#include "NetConnectionConnectCommand.h"

using namespace Mntone::Rtmp::Command;

NetConnectionConnectCommand::NetConnectionConnectCommand( Platform::String^ app ) :
	_App( app ),
	_FlashVersion( "WIN 9,0,262,0" ),
	_SwfUrl( "http://localhost/dummy.swf" ),
	_Fpad( false ),
	_AudioCodecs( SupportSoundType::Mp3 | SupportSoundType::Aac ),
	_VideoCodecs( SupportVideoType::Sorenson | SupportVideoType::H264 ),
	_VideoFunction( SupportVideoFunctionType::Seek ),
	_PageUrl( "http://localhost/dummy.html" ),
	_ObjectEncoding( Mntone::Data::Amf::AmfEncodingType::Amf0 )
{ }

Mntone::Data::Amf::AmfArray^ NetConnectionConnectCommand::Commandify( void )
{
	using namespace Mntone::Data::Amf;
	auto a = ref new AmfArray();
	a->Append( AmfValue::CreateStringValue( "connect" ) );	// Command name
	a->Append( AmfValue::CreateDoubleValue( 1.0 ) );		// Transaction id: always set to 1.

	auto obj = ref new AmfObject();
	obj->Insert( "app", AmfValue::CreateStringValue( _App ) );
	obj->Insert( "flashVer", AmfValue::CreateStringValue( _FlashVersion ) );
	obj->Insert( "swfUrl", AmfValue::CreateStringValue( _SwfUrl ) );
	obj->Insert( "tcUrl", AmfValue::CreateStringValue( _TcUrl ) );
	obj->Insert( "fpad", AmfValue::CreateBooleanValue( _Fpad ) );
	obj->Insert( "audioCodecs", AmfValue::CreateDoubleValue( static_cast<double>( _AudioCodecs ) ) );
	obj->Insert( "videoCodecs", AmfValue::CreateDoubleValue( static_cast<double>( _VideoCodecs ) ) );
	obj->Insert( "videoFunction", AmfValue::CreateDoubleValue( static_cast<double>( _VideoFunction ) ) );
	obj->Insert( "pageUrl", AmfValue::CreateStringValue( _PageUrl ) );
	obj->Insert( "objectEncoding", AmfValue::CreateDoubleValue( static_cast<double>( _ObjectEncoding ) ) );
	a->Append( obj );

	if( _OptionalUserArguments != nullptr )
		a->Append( _OptionalUserArguments );
	else
		a->Append( ref new AmfValue() );

	return a;
}

Platform::String^ NetConnectionConnectCommand::ToString( void )
{
	return "connect (" + _App + ")";
}