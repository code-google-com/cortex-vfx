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

/// ParameterHandlers are responsible for mapping between IECore::Parameters
/// and DD::Image::Knobs.
class ParameterHandler : public IECore::RefCounted
{

	public :
		
		enum ValueSource
		{
			Knob,
			Storage
		};
		
		/// Declares knobs to represent the Parameter.
		virtual void knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f ) = 0;
		/// Transfers the value from Nuke onto the Parameter. ValueSource may be passed
		/// a value of Knob if it is known that Nuke hasn't stored knob
		/// values yet - for instance in a knob_changed() method with
		/// a KNOB_CHANGED_ALWAYS knob. This causes the value to be retrieved
		/// directly from the knob at the current time, rather than from the
		/// value stored by the knob.
		virtual void setParameterValue( IECore::Parameter *parameter, ValueSource valueSource = Storage ) = 0;
		
		/// Transfers the value from the Parameter back onto the nuke knob at the current time.
		virtual void setKnobValue( const IECore::Parameter *parameter ) = 0;
		
		/// Factory function to create a ParameterHandler suitable for a given Parameter.
		static ParameterHandlerPtr create( const IECore::Parameter *parameter );
		
	protected :
	
		ParameterHandler();
	
		const char *knobLabel( const IECore::Parameter *parameter ) const;
		
		template<typename T>
		class Description
		{
			public :
			
				Description( IECore::TypeId parameterType )
				{
					creatorFns()[parameterType] = creator;
				}

			private :
				
				static ParameterHandlerPtr creator()
				{
					return new T();
				}
					
		};

	private :
			
		typedef ParameterHandlerPtr (*CreatorFn)();
	
		typedef std::map<IECore::TypeId, CreatorFn> CreatorFnMap;
		static CreatorFnMap &creatorFns();
		
};

} // namespace IECoreNuke

#endif // IECORENUKE_PARAMETERHANDLER_H
