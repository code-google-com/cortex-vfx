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

#ifndef IECORENUKE_PARAMETERISEDHOLDER_H
#define IECORENUKE_PARAMETERISEDHOLDER_H

// Forward declaration of Nuke stuff

class Node; // Don't know why this isn't in a namespace
namespace DD
{
namespace Image
{
	class Op;
} // namespace Image
} // namespace DD

#include "IECore/Parameter.h"
#include "IECore/Parameterised.h"

#include "IECoreNuke/ParameterHandler.h"
#include "IECoreNuke/ObjectKnob.h"

namespace IECoreNuke
{

template<typename BaseType>
class ParameterisedHolder : public BaseType
{

	public :

		ParameterisedHolder( Node *node );
		virtual ~ParameterisedHolder();

		//! @name Reimplementation of functions defined by the Nuke BaseType.
		/////////////////////////////////////////////////////////////////////
		//@{
		/// Calls knobs() on a ParameterHandler to represent the Parameters.
		virtual void knobs( DD::Image::Knob_Callback f );
		/// Implemented to load the Parameterised class.
		virtual int knob_changed( DD::Image::Knob *knob );
		//@}
	
		/// \todo Consider how these should fit in really.
		/// Should they be protected?
		IECore::RunTimeTypedPtr getParameterised();
		void setParameterValues();
		
	private :

		// class specification
		IECore::ObjectPtr m_classSpecifier;
		ObjectKnob *m_classSpecifierKnob;
		DD::Image::Knob *m_classReloadKnob; // for triggering reloading as a user
		
		// class loading
		IECore::RunTimeTypedPtr m_parameterised;
		IECore::RunTimeTypedPtr loadClass( bool refreshLoader );

		// knob creation
		ParameterHandlerPtr m_parameterHandler;
		size_t m_numParameterKnobs;
		static void parameterKnobs( void *that, DD::Image::Knob_Callback f );
		
};

typedef ParameterisedHolder<DD::Image::Op> ParameterisedHolderOp;

} // namespace IECoreNuke

#endif // IECORENUKE_PARAMETERISEDHOLDER_H
