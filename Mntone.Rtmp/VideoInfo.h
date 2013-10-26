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
			VideoFormat get( void ) { return _Format; }
		internal:
			void set( VideoFormat value ) { _Format = value; }
		}

	private:
		VideoFormat _Format;
	};

} }