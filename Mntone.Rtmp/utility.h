#pragma once

namespace mntone { namespace rtmp { namespace utility {

	void convert_big_endian( const void* first, size_t size, void* dest );
	void convert_little_endian( const void *const first, const size_t size, void *const dest );

	uint64 windows_time_to_unix_time( const int64 windows_time );
	int64 unix_time_to_windows_time( const uint64 unix_time );

	Windows::Foundation::DateTime get_date_time();
	int64 get_windows_time();

	uint32 hundred_nano_to_milli( int64 hundred_nano );

} } }