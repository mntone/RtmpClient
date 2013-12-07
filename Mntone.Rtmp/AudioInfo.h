#pragma once
#include "AudioFormat.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AudioInfo sealed
	{
	internal:
		AudioInfo() { }

	public:
		property AudioFormat Format
		{
			AudioFormat get() { return Format_; }
		internal:
			void set( AudioFormat value ) { Format_ = value; }
		}
		property uint32 SampleRate
		{
			uint32 get() { return SampleRate_; }
		internal:
			void set( uint32 value ) { SampleRate_ = value; }
		}
		property uint16 ChannelCount
		{
			uint16 get() { return ChannelCount_; }
		internal:
			void set( uint16 value ) { ChannelCount_ = value; }
		}
		property uint16 Bitrate
		{
			uint16 get() { return Bitrate_; }
		internal:
			void set( uint16 value ) { Bitrate_ = value; }
		}
		property uint16 BitsPerSample
		{
			uint16 get() { return BitsPerSample_; }
		internal:
			void set( uint16 value ) { BitsPerSample_ = value; }
		}

	private:
		AudioFormat Format_;
		uint32 SampleRate_;
		uint16 ChannelCount_, Bitrate_, BitsPerSample_;
	};

} }