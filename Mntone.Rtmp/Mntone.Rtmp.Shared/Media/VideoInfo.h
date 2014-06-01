#pragma once
#include "VideoFormat.h"
#include "AvcProfileIndication.h"

namespace Mntone { namespace Rtmp { namespace Media {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class VideoInfo sealed
	{
	internal:
		VideoInfo() { }

	public:
		property VideoFormat Format
		{
			VideoFormat get() { return Format_; }
		internal:
			void set( VideoFormat value ) { Format_ = value; }
		}
		property AvcProfileIndication ProfileIndication
		{
			AvcProfileIndication get() { return ProfileIndication_; }
		internal:
			void set( AvcProfileIndication value ) { ProfileIndication_ = value; }
		}
		property uint16 Bitrate
		{
			uint16 get() { return Bitrate_; }
		internal:
			void set( uint16 value ) { Bitrate_ = value; }
		}
		property uint16 Height
		{
			uint16 get() { return Height_; }
		internal:
			void set( uint16 value ) { Height_ = value; }
		}
		property uint16 Width
		{
			uint16 get() { return Width_; }
		internal:
			void set( uint16 value ) { Width_ = value; }
		}

	private:
		VideoFormat Format_;
		AvcProfileIndication ProfileIndication_;
		uint16 Bitrate_, Height_, Width_;
	};

} } }