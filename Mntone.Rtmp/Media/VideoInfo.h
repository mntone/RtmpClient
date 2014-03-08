#pragma once
#include "VideoFormat.h"

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

	private:
		VideoFormat Format_;
	};

} } }