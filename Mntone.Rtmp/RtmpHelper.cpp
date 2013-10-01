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