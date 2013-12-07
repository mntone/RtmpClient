#pragma once
#include "VideoFormat.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class VideoInfo sealed
	{
	internal:
		VideoInfo( void ) { }

	public:
		property VideoFormat Format
		{
			VideoFormat get( void ) { return Format_; }
		internal:
			void set( VideoFormat value ) { Format_ = value; }
		}

	private:
		VideoFormat Format_;
	};

} }