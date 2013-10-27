#pragma once
#include "NetStatusCodeType.h"

namespace Mntone { namespace Rtmp {

	ref class RtmpHelper sealed
	{
	internal:
		static Mntone::Data::Amf::AmfArray^ ParseAmf0( std::vector<uint8> data );
		static Mntone::Data::Amf::AmfArray^ ParseAmf3( std::vector<uint8> data );

		static NetStatusCodeType ParseNetConnectionConnectCode( const std::wstring code );
		static NetStatusCodeType ParseNetStreamCode( const std::wstring code );
	};

} }