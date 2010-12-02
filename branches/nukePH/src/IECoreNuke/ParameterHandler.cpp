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

#include "IECoreNuke/ParameterHandler.h"
#include "IECoreNuke/PresetsOnlyParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::ParameterHandler( IECore::ParameterPtr parameter, const std::string &knobName )
	:	m_parameter( parameter ), m_knobName( knobName )
{
}

void ParameterHandler::setParameterValue( ValueSource valueSource )
{
	setParameterValue( parameter(), valueSource );
}

ParameterHandlerPtr ParameterHandler::create( IECore::ParameterPtr parameter, const std::string &knobName )
{
	if( parameter->presetsOnly() )
	{
		return new PresetsOnlyParameterHandler( parameter, knobName );
	}

	const CreatorFnMap &creators = creatorFns();
	TypeId typeId = parameter->typeId();
	while( typeId!=InvalidTypeId )
	{
		CreatorFnMap::const_iterator it = creators.find( typeId );
		if( it!=creators.end() )
		{
			return it->second( parameter, knobName );
		}	
		typeId = RunTimeTyped::baseTypeId( typeId );
	}
	return 0;
}

ParameterHandler::CreatorFnMap &ParameterHandler::creatorFns()
{
	static CreatorFnMap creators;
	return creators;
}

IECore::Parameter *ParameterHandler::parameter() const
{
	return m_parameter.get();
}

const char *ParameterHandler::knobName() const
{
	return m_knobName.value().c_str();
}

const char *ParameterHandler::knobLabel() const
{
	/// \todo Implement some nice camel case based string formatting
	return m_parameter->name().c_str();
}
