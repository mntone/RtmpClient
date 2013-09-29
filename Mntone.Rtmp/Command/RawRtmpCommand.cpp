#include "pch.h"
#include "RawRtmpCommand.h"

using namespace Mntone::Rtmp::Command;

RawRtmpCommand::RawRtmpCommand( Platform::String^ type, Mntone::Data::Amf::AmfArray^ command ) :
	_Type( type ),
	_command( command )
{ }

Mntone::Data::Amf::AmfArray^ RawRtmpCommand::Commandify( void )
{
	return _command;
}