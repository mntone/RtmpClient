#include "pch.h"
#include "Utilities.h"

namespace Mntone { namespace Rtmp {

	void ConvertBigEndian( const void *const from, void *const to, const size_t size )
	{
		auto const from_ptr = reinterpret_cast<const uint8 *>( from );
		auto const to_ptr = reinterpret_cast<uint8 *>( to );

		for( auto i = 0u; i < size; ++i )
			to_ptr[i] = from_ptr[size - i - 1];
	}

	void ConvertLittleEndian( const void *const from, void *const to, const size_t size )
	{
		memcpy( to, from, size );
	}

	Platform::String^ CharUtf8ToPlatformString( const std::string& charUtf8 )
	{
		const auto charUtf8Length = static_cast<int32>( charUtf8.length() );
		const auto length = MultiByteToWideChar( CP_UTF8, 0, charUtf8.c_str(), charUtf8Length, nullptr, 0 );
		const auto bufferLength = length + 1;
		std::vector<char16> buffer( bufferLength );
		if( MultiByteToWideChar( CP_UTF8, 0, charUtf8.c_str(), charUtf8Length, buffer.data(), bufferLength ) == 0 )
			throw ref new Platform::InvalidArgumentException( "Invaild text" );
		return ref new Platform::String( buffer.data() );
	}

	std::string PlatformStringToCharUtf8( Platform::String^ platformString )
	{
		const auto length = WideCharToMultiByte( CP_UTF8, 0, platformString->Data(), platformString->Length(), nullptr, 0, nullptr, nullptr );
		const auto bufferLength = length + 1;
		std::vector<char> buffer( bufferLength );
		if( WideCharToMultiByte( CP_UTF8, 0, platformString->Data(), platformString->Length(), buffer.data(), bufferLength, nullptr, nullptr ) == 0 )
			throw ref new Platform::InvalidArgumentException( "Invaild text" );
		return std::string( buffer.data() );
	}

	uint64 WindowsTimeToUnixTime( const int64 windowsTime )
	{
		return static_cast<uint64>( ( windowsTime - 116444736000000000ll ) / 10000000ll );
	}

	int64 UnixTimeToWindowsTime( const uint64 unixTime )
	{
		return 10000000ll * static_cast<uint64>( unixTime ) + 116444736000000000ll;
	}

	uint64 DateTimeToUnixTime( Windows::Foundation::DateTime dateTime )
	{
		return WindowsTimeToUnixTime( dateTime.UniversalTime );
	}

	Windows::Foundation::DateTime UnixTimeToDateTime( const uint64 unixTime )
	{
		Windows::Foundation::DateTime d;
		d.UniversalTime = UnixTimeToWindowsTime( unixTime );
		return d;
	}

	Windows::Foundation::DateTime GetDateTime( void )
	{
		//auto calendar = ref new Windows::Globalization::Calendar();
		//calendar->SetToNow();
		//return calendar->GetDateTime();

		Windows::Foundation::DateTime dt;
		dt.UniversalTime = GetWindowsTime();
		return dt;
	}

	int64 GetWindowsTime( void )
	{
		//return GetDateTime().UniversalTime;

		FILETIME ft;
		GetSystemTimeAsFileTime( &ft );

		return static_cast<uint64>( ft.dwHighDateTime ) << 32 | static_cast<uint64>( ft.dwLowDateTime );
	}

	uint32 HundredNanoToMilli( int64 hundredNano )
	{
		return static_cast<uint32>( hundredNano / 10000000ll );
	}

} }
