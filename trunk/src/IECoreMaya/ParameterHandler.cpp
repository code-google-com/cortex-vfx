//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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


#include "IECoreMaya/ParameterHandler.h"

using namespace IECoreMaya;

ParameterHandler::~ParameterHandler()
{
}

ConstParameterHandlerPtr ParameterHandler::get( IECore::ConstParameterPtr parameter )
{
	return get( parameter->typeId() );
}

ConstParameterHandlerPtr ParameterHandler::get( IECore::ConstObjectPtr object )
{
	return get( object->typeId() );
}

ConstParameterHandlerPtr ParameterHandler::get( IECore::TypeId id )
{
	const HandlerMap &h = handlers();
	
	do {
		HandlerMap::const_iterator it = h.find( id );
		if( it!=h.end() )
		{
			return it->second;
		}
		id = IECore::RunTimeTyped::baseTypeId( id );
	} while( id!=IECore::InvalidTypeId );
	
	return 0;
}

ParameterHandler::HandlerMap &ParameterHandler::handlers()
{
	static HandlerMap *g_handlers = new HandlerMap();

	return *g_handlers;
}

void ParameterHandler::registerHandler( IECore::TypeId parameterType, IECore::TypeId dataType, ConstParameterHandlerPtr handler )
{
	assert( parameterType != IECore::InvalidTypeId );
	handlers()[parameterType] = handler;

	if ( dataType != IECore::InvalidTypeId )
	{
		handlers()[dataType] = handler;
	}
}
