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

#include "boost/algorithm/string/replace.hpp"

#include "DDImage/Knobs.h"
#include "DDImage/Enumeration_KnobI.h"

#include "IECore/MessageHandler.h"
#include "IECore/CompoundData.h"
#include "IECore/SimpleTypedData.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreNuke/ClassParameterHandler.h"

using namespace boost;
using namespace std;
using namespace boost::python;
using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<ClassParameterHandler> ClassParameterHandler::g_description( ClassParameterTypeId );

ClassParameterHandler::ClassParameterHandler()
{
}
		
void ClassParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{	
	DD::Image::BeginClosedGroup( f, knobName, knobLabel( parameter ) );
		
		classChooserKnob( parameter, knobName, f );
		
		childKnobs( parameter, knobName, f );

	DD::Image::EndGroup( f );
}

void ClassParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	CompoundParameterHandler::setParameterValue( parameter, valueSource );
}

void ClassParameterHandler::setState( IECore::Parameter *parameter, const IECore::Object *state )
{
	const CompoundData *d = static_cast<const CompoundData *>( state );
	
	const std::string &className = d->member<StringData>( "className" )->readable();
	int classVersion = d->member<IntData>( "classVersion" )->readable();
	const std::string &classSearchPathEnvVar = d->member<StringData>( "searchPathEnvVar" )->readable();

	/// \todo C++ code shouldn't have to call python explicity to do this stuff.
	/// We could define an abstract C++ class with the right interface and then derive
	/// from it to implement it in python.
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( ParameterPtr( const_cast<Parameter *>( parameter ) ) );
		pythonParameter.attr( "setClass" )( className, classVersion, classSearchPathEnvVar );
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "ClassParameterHandler::updateClass", e.what() );
	}	
}

IECore::ObjectPtr ClassParameterHandler::getState( const IECore::Parameter *parameter )
{
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( ParameterPtr( const_cast<Parameter *>( parameter ) ) );
		boost::python::tuple classInfo = extract<boost::python::tuple>( pythonParameter.attr( "getClass" )( true ) );
		
		std::string className = extract<const char *>( classInfo[1] )();
		int classVersion = extract<int>( classInfo[2] )();
		std::string searchPathEnvVar = extract<const char *>( classInfo[3] )();
		
		IECore::CompoundDataPtr result = new IECore::CompoundData();
		result->writable()["className"] = new IECore::StringData( className );
		result->writable()["classVersion"] = new IECore::IntData( classVersion );
		result->writable()["searchPathEnvVar"] = new IECore::StringData( searchPathEnvVar );
		
		return result;
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "ClassParameterHandler::getClass", e.what() );
	}
	return 0;
}

void ClassParameterHandler::classChooserKnob( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{

	std::string classChooserName = string( knobName ) + "__classChooser";
	
	static const char *noVersions[] = { "No class loaded", "", 0 };
	DD::Image::Knob *classChooser = PyPulldown_knob( f, noVersions, classChooserName.c_str(), "No class loaded" );
	
	vector<string> menuItems;
	IECorePython::ScopedGILLock gilLock;
	try
	{
		object mainModule = import( "__main__" );
		object mainModuleNamespace = mainModule.attr( "__dict__" );
	
		boost::python::object pythonParameter( ParameterPtr( const_cast<Parameter *>( parameter ) ) );
		boost::python::tuple classInfo = extract<boost::python::tuple>( pythonParameter.attr( "getClass" )( true ) );

		std::string searchPathEnvVar = extract<const char *>( classInfo[3] )();
		
		object ieCore = import( "IECore" );
		object classLoader = ieCore.attr( "ClassLoader" ).attr( "defaultLoader" )( searchPathEnvVar );
		
		object classNames = classLoader.attr( "classNames" )();
		
		int numClasses = len( classNames );
		for( int i=0; i<numClasses; i++ )
		{
			string className = extract<string>( classNames[i] )();
			object classVersions = classLoader.attr( "versions" )( object( classNames[i] ) );
			int numVersions = len( classVersions );
			
			for( int j=0; j<numVersions; j++ )
			{
				string versionString = extract<const char *>( classVersions[j].attr( "__str__" )() )();
				if( numVersions > 1 )
				{
					/// \todo We need to make this menu nice and hierarchical.
					/// We need the nuke boys to sort that out though.
					menuItems.push_back( className + " v" + versionString );
				}
				else
				{
					menuItems.push_back( className );
				}
				
				std::string parameterPath = knobName + 5; // naughty! we're not meant to know the knob name format
				replace_all( parameterPath, "_", "\"][\"" );
				
				std::string cmd = ( boost::format(
				
					"with IECoreNuke.FnParameterisedHolder( nuke.thisNode() ).parameterModificationContext() as parameters :"
					"	parameters['\%s'].setClass( '%s', %d )"
					
				) % parameterPath % className % versionString ).str();
					
				menuItems.push_back( cmd );
			}
			
		}
		
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "ClassParameterHandler::classChooserKnob", e.what() );
	}
	
	classChooser->enumerationKnob()->menu( menuItems );
}
