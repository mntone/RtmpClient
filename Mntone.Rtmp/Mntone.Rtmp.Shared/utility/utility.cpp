#include "pch.h"
#include "utility.h"

using namespace mntone::rtmp;

void utility::convert_big_endian( const void* const first, size_t size, void* const dest )
{
	auto ptr = reinterpret_cast<const uint8*>( first ) + size;
	auto pdest = reinterpret_cast<uint8*>( dest );

	for( ; first != ptr; ++pdest )
	{
		*pdest = *--ptr;
	}
}

void utility::convert_little_endian( const void *const first, const size_t size, void *const dest )
{
	memcpy( dest, first, size );
}

uint64 utility::windows_time_to_unix_time( const int64 windows_time )
{
	return ( windows_time - 116444736000000000ull ) / 10000000ull;
}

int64 utility::unix_time_to_windows_time( const uint64 unix_time )
{
	return 10000000ll * unix_time + 116444736000000000ll;
}

Windows::Foundation::DateTime utility::get_date_time()
{
	Windows::Foundation::DateTime dt;
	dt.UniversalTime = get_windows_time();
	return dt;
}

int64 utility::get_windows_time()
{
	FILETIME ft;
	GetSystemTimeAsFileTime( &ft );
	const auto& li = *reinterpret_cast<LARGE_INTEGER*>( &ft );
	return li.QuadPart;
}

uint32 utility::hundred_nano_to_milli( int64 hundred_nano )
{
	return static_cast<uint32>( hundred_nano / 10000000ll );
}