#include "pch.h"
#include "RawRtmpCommand.h"

namespace Mntone { namespace Rtmp { namespace Command {

	RawRtmpCommand::RawRtmpCommand( Platform::String^ type, Mntone::Data::Amf::AmfArray^ command ):
		_Type( type ),
		_command( command )
	{ }

	Mntone::Data::Amf::AmfArray^ RawRtmpCommand::Commandify( void )
	{
		return _command;
	}

} } }