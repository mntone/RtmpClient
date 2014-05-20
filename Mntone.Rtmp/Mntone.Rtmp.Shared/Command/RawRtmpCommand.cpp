#include "pch.h"
#include "RawRtmpCommand.h"

using namespace Mntone::Rtmp::Command;

RawRtmpCommand::RawRtmpCommand( Platform::String^ type, Mntone::Data::Amf::AmfArray^ command )
	: Type_( type )
	, command_( command )
{ }

Mntone::Data::Amf::AmfArray^ RawRtmpCommand::Commandify()
{
	return command_;
}