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

#ifndef IECORENUKE_PARAMETERHANDLER_H
#define IECORENUKE_PARAMETERHANDLER_H

#include "DDImage/Op.h" // for Knob_Callback

#include "IECore/Parameter.h"
#include "IECore/Interned.h"

namespace IECoreNuke
{

IE_CORE_FORWARDDECLARE( ParameterHandler )

class ParameterHandler : public IECore::RefCounted
{

	public :
				
		ParameterHandler( IECore::ParameterPtr parameter, const std::string &knobName );
		
		/// Declares knobs to represent the Parameter.
		virtual void knobs( DD::Image::Knob_Callback f ) = 0;
		/// Transfers the value from Nuke onto the Parameter.
		virtual void setParameterValue() = 0;
		
		static ParameterHandlerPtr create( IECore::ParameterPtr parameter, const std::string &knobName );
		
	protected :
	
		IECore::ParameterPtr parameter() const;
		const char *knobName() const;
		const char *knobLabel() const;
		
		template<typename T>
		class Description
		{
			public :
			
				Description( IECore::TypeId parameterType )
				{
					creatorFns()[parameterType] = creator;
				}

			private :
				
				static ParameterHandlerPtr creator( IECore::ParameterPtr parameter, const std::string &knobName  )
				{
					return new T( parameter, knobName );
				}
					
		};

	private :
	
		const IECore::ParameterPtr m_parameter;
		const IECore::InternedString m_knobName;
		
		typedef ParameterHandlerPtr (*CreatorFn)( IECore::ParameterPtr parameter, const std::string &knobName  );
	
		typedef std::map<IECore::TypeId, CreatorFn> CreatorFnMap;
		static CreatorFnMap &creatorFns();
		
};

} // namespace IECoreNuke

#endif // IECORENUKE_PARAMETERHANDLER_H
