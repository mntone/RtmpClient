#include "pch.h"
#include "NetConnectionCallCommand.h"

using namespace Mntone::Rtmp::Command;

NetConnectionCallCommand::NetConnectionCallCommand( Platform::String^ commandName )
	: CommandName_( commandName )
	, TransactionId_( 0 )
	, CommandObject_( nullptr )
	, OptionalArguments_( ref new Platform::Collections::Vector<Mntone::Data::Amf::IAmfValue^>() )
{ }

Mntone::Data::Amf::AmfArray^ NetConnectionCallCommand::Commandify()
{
	using namespace Mntone::Data::Amf;

	auto ary = ref new AmfArray();
	ary->Append( AmfValue::CreateStringValue( CommandName_ ) );								// Command name
	ary->Append( AmfValue::CreateNumberValue( static_cast<float64>( TransactionId_ ) ) );	// Transaction id
	ary->Append( CommandObject_ != nullptr ? CommandObject_ : ref new AmfValue() );
	if( OptionalArguments_->Size != 0 )
	{
		for( auto&& optionalArgument : OptionalArguments_ )
		{
			ary->Append( optionalArgument );
		}
	}
	else
	{
		ary->Append( ref new AmfValue() );
	}
	return ary;
}

Platform::String^ NetConnectionCallCommand::ToString()
{
	return CommandName_;
}