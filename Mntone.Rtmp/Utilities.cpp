#include "pch.h"
#include "Utilities.h"

void Mntone::Rtmp::ConvertBigEndian( const void *const from, void *const to, const size_t size )
{
	auto const from_ptr = reinterpret_cast<const uint8 *>( from );
	auto const to_ptr = reinterpret_cast<uint8 *>( to );

	for( auto i = 0u; i < size; ++i )
		to_ptr[i] = from_ptr[size - i - 1];
}

void Mntone::Rtmp::ConvertLittleEndian( const void *const from, void *const to, const size_t size )
{
	memcpy( to, from, size );
}

uint64 Mntone::Rtmp::WindowsTimeToUnixTime( const int64 windowsTime )
{
	return ( windowsTime - 116444736000000000ull ) / 10000000ull;
}

int64 Mntone::Rtmp::UnixTimeToWindowsTime( const uint64 unixTime )
{
	return 10000000ll * unixTime + 116444736000000000ll;
}

Windows::Foundation::DateTime Mntone::Rtmp::GetDateTime( void )
{
	Windows::Foundation::DateTime dt;
	dt.UniversalTime = ::Mntone::Rtmp::GetWindowsTime();
	return dt;
}

int64 Mntone::Rtmp::GetWindowsTime( void )
{
	FILETIME ft;
	GetSystemTimeAsFileTime( &ft );
	const auto& li = *reinterpret_cast<LARGE_INTEGER*>( &ft );
	return li.QuadPart;
}

uint32 Mntone::Rtmp::HundredNanoToMilli( int64 hundredNano )
{
	return static_cast<uint32>( hundredNano / 10000000ll );
}