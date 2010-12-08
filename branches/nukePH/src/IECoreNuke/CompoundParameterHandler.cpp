//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "DDImage/Knobs.h"

#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

#include "IECoreNuke/CompoundParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<CompoundParameterHandler> CompoundParameterHandler::g_description( CompoundParameter::staticTypeId() );

CompoundParameterHandler::CompoundParameterHandler( )
{
}
		
void CompoundParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{	
	if( strcmp( knobName, "parm" ) ) // only make a group if non-top-level parameters
	{
		DD::Image::BeginClosedGroup( f, knobName, knobLabel( parameter ) );
	}

		childKnobs( parameter, knobName, f );
	
	if( strcmp( knobName, "parm" ) )
	{
		DD::Image::EndGroup( f );
	}
}

void CompoundParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	CompoundParameter *compoundParameter = static_cast<CompoundParameter *>( parameter );
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), false );
		if( h )
		{
			h->setParameterValue( cIt->get(), valueSource );
		}
	}
}

void CompoundParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), false );
		if( h )
		{
			h->setKnobValue( cIt->get() );
		}
	}
}

void CompoundParameterHandler::setState( IECore::Parameter *parameter, const IECore::Object *state )
{
	const CompoundObject *o = static_cast<const CompoundObject *>( state );
	CompoundParameter *compoundParameter = static_cast<CompoundParameter *>( parameter );
	
	const CompoundObject::ObjectMap &members = o->members();
	for( CompoundObject::ObjectMap::const_iterator it = members.begin(); it!=members.end(); it++ )
	{
		Parameter *child = compoundParameter->parameter<Parameter>( it->first );
		if( child )
		{
			ParameterHandlerPtr h = handler( child, true );
			if( h )
			{
				h->setState( child, it->second );
			}
		}
	}
}

IECore::ObjectPtr CompoundParameterHandler::getState( const IECore::Parameter *parameter )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );
	CompoundObjectPtr result = new CompoundObject;
	
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( *cIt, false );
		if( h )
		{
			ObjectPtr childState = h->getState( *cIt );
			if( childState )
			{
				result->members()[(*cIt)->name()] = childState;
			}
		}
	}
	
	if( result->members().size() )
	{
		return result;
	}
	
	return 0;
}
		
void CompoundParameterHandler::childKnobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );

	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( *cIt, true );
		if( h )
		{
			std::string childKnobName = std::string( knobName ) + "_" + (*cIt)->name();
			h->knobs( cIt->get(), childKnobName.c_str(), f );
		}
	}
}

ParameterHandlerPtr CompoundParameterHandler::handler( const Parameter *child, bool createIfMissing )
{
	HandlerMap::const_iterator it = m_handlers.find( child->internedName() );
	if( it!=m_handlers.end() )
	{
		return it->second;
	}
	
	if( !createIfMissing )
	{
		return 0;
	}
	
	ParameterHandlerPtr h = ParameterHandler::create( child );
	if( !h )
	{
		IECore::msg( IECore::Msg::Warning, "IECoreNuke::CompoundParameterHandler", boost::format(  "Unable to create handler for parameter \"%s\" of type \"%s\"" ) % child->name() % child->typeName() );
	}
	
	m_handlers[child->internedName()] = h;
	return h;
}
