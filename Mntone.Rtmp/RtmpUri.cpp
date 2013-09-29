#include "pch.h"
#include "RtmpUri.h"

using namespace Mntone::Rtmp;

RtmpUri::RtmpUri( Platform::String^ uri )
{
	RtmpUri( ref new Windows::Foundation::Uri( uri ) );
}

RtmpUri::RtmpUri( Windows::Foundation::Uri^ uri )
{
	ParseScheme( uri->SchemeName );
	_Host = uri->Host;
	ParsePort( uri->Port );
	ParsePath( uri->Path );
}

void RtmpUri::ParseScheme( Platform::String^ schemeName )
{
	if( schemeName->Equals( "rtmp" ) )
	{
		_Scheme = RtmpScheme::Rtmp;
		return;
	}

	throw ref new Platform::InvalidArgumentException( "Invalid scheme name." );
}

void RtmpUri::ParsePort( int32 port )
{
	if( port == 0 )
	{
		switch( _Scheme )
		{
		case RtmpScheme::Rtmp: _Port = 1935; break;
		case RtmpScheme::Rtmps: _Port = 443; break;
		case RtmpScheme::Rtmpe: _Port = 1935; break;
		case RtmpScheme::Rtmpt: _Port = 80; break;
		case RtmpScheme::Rtmpte: _Port = 80; break;
		default: throw ref new Platform::InvalidArgumentException( "Invalid port." );
		}
	}
	else
		_Port = port;
}

void RtmpUri::ParsePath( Platform::String^ path )
{
	std::wstring p( path->Data() );

	if( p[0] != '/' )
		throw ref new Platform::InvalidArgumentException( "Invalid path." );

	auto spos1 = p.find( L'/', 1 );
	if( spos1 == std::wstring::npos )
	{
		auto app = p.substr( 1 );
		_App = ref new Platform::String( app.c_str(), static_cast<uint32>( app.length() ) );
		return;
	}

	auto app = p.substr( 1, spos1 - 1 );
	_App = ref new Platform::String( app.c_str(), static_cast<uint32>( app.length() ) );

	auto spos2 = p.find( L'/', spos1 );
	auto instance = p.substr( spos1 + 1, spos2 );
	_Instance = ref new Platform::String( instance.c_str(), static_cast<uint32>( instance.length() ) );
}

Platform::String^ RtmpUri::ToString( void )
{
	std::wstringstream buf;

	auto scheme = _Scheme.ToString();
	buf.write( scheme->Data(), scheme->Length() );

	buf.write( L"://", 3 );

	auto port = _Port.ToString();
	buf.write( port->Data(), port->Length() );

	buf.put( '/' );
	buf.write( _App->Data(), _App->Length() );

	if( !_Instance->IsEmpty() )
	{
		buf.put( '/' );
		buf.write( _Instance->Data(), _Instance->Length() );
	}

	auto str = buf.str();
	return ref new Platform::String( str.c_str(), static_cast<uint32>( str.length() ) );
}