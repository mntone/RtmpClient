#pragma once
#include "AudioFormat.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AudioInfo sealed
	{
	internal:
		AudioInfo( void ) { }

	public:
		property AudioFormat Format
		{
			AudioFormat get( void ) { return _Format; }
		internal:
			void set( AudioFormat value ) { _Format = value; }
		}
		property uint32 SampleRate
		{
			uint32 get( void ) { return _SampleRate; }
		internal:
			void set( uint32 value ) { _SampleRate = value; }
		}
		property uint16 ChannelCount
		{
			uint16 get( void ) { return _ChannelCount; }
		internal:
			void set( uint16 value ) { _ChannelCount = value; }
		}
		property uint16 Bitrate
		{
			uint16 get( void ) { return _Bitrate; }
		internal:
			void set( uint16 value ) { _Bitrate = value; }
		}
		property uint16 BitsPerSample
		{
			uint16 get( void ) { return _BitsPerSample; }
		internal:
			void set( uint16 value ) { _BitsPerSample = value; }
		}

	private:
		AudioFormat _Format;
		uint32 _SampleRate;
		uint16 _ChannelCount, _Bitrate, _BitsPerSample;
	};

} }