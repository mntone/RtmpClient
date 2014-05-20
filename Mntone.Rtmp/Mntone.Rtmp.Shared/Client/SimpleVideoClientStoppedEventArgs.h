#pragma once

namespace Mntone { namespace Rtmp { namespace Client {

	[Windows::Foundation::Metadata::Threading( Windows::Foundation::Metadata::ThreadingModel::Both )]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SimpleVideoClientStoppedEventArgs sealed
	{
	internal:
		SimpleVideoClientStoppedEventArgs();
	};

} } }