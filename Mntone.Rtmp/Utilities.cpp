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
	return static_cast<uint64>( ( windowsTime - 116444736000000000ll ) / 10000000ll );
}

int64 Mntone::Rtmp::UnixTimeToWindowsTime( const uint64 unixTime )
{
	return 10000000ll * static_cast<uint64>( unixTime ) + 116444736000000000ll;
}

uint64 Mntone::Rtmp::DateTimeToUnixTime( Windows::Foundation::DateTime dateTime )
{
	return ::Mntone::Rtmp::WindowsTimeToUnixTime( dateTime.UniversalTime );
}

Windows::Foundation::DateTime Mntone::Rtmp::UnixTimeToDateTime( const uint64 unixTime )
{
	Windows::Foundation::DateTime d;
	d.UniversalTime = ::Mntone::Rtmp::UnixTimeToWindowsTime( unixTime );
	return d;
}

Windows::Foundation::DateTime Mntone::Rtmp::GetDateTime( void )
{
	//auto calendar = ref new Windows::Globalization::Calendar();
	//calendar->SetToNow();
	//return calendar->GetDateTime();

	Windows::Foundation::DateTime dt;
	dt.UniversalTime = ::Mntone::Rtmp::GetWindowsTime();
	return dt;
}

int64 Mntone::Rtmp::GetWindowsTime( void )
{
	//return GetDateTime().UniversalTime;

	FILETIME ft;
	GetSystemTimeAsFileTime( &ft );

	return static_cast<uint64>( ft.dwHighDateTime ) << 32 | static_cast<uint64>( ft.dwLowDateTime );
}

uint32 Mntone::Rtmp::HundredNanoToMilli( int64 hundredNano )
{
	return static_cast<uint32>( hundredNano / 10000000ll );
}