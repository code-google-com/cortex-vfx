//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"
#include "boost/format.hpp"

#include "maya/MSyntax.h"
#include "maya/MArgParser.h"
#include "maya/MStringArray.h"
#include "maya/MFnDagNode.h"
#include "maya/MSelectionList.h"

#include "IECore/CompoundParameter.h"
#include "IECore/AttributeBlock.h"

#include "IECoreRI/Renderer.h"
#include "IECoreRI/Convert.h"

#include "IECoreMaya/DelightProceduralCacheCommand.h"
#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/PythonCmd.h"
#include "IECoreMaya/Convert.h"

#include "ri.h"

#define STRINGIFY( ARG ) STRINGIFY2( ARG )
#define STRINGIFY2( ARG ) #ARG

using namespace boost::python;
using namespace IECoreMaya;

DelightProceduralCacheCommand::ProceduralMap DelightProceduralCacheCommand::g_procedurals;

DelightProceduralCacheCommand::DelightProceduralCacheCommand()
{
}

DelightProceduralCacheCommand::~DelightProceduralCacheCommand()
{
}

void *DelightProceduralCacheCommand::creator()
{
	return new DelightProceduralCacheCommand;
}

MSyntax DelightProceduralCacheCommand::newSyntax()
{
	MSyntax syn;
	MStatus s;
  
	s = syn.addFlag( "-a", "-addstep" );
	assert(s);
	
	s = syn.addFlag( "-e", "-emit" );
	assert(s);
	
	s = syn.addFlag( "-f", "-flush" );
	assert(s);
	
	s = syn.addFlag( "-r", "-remove" );
	assert(s);
	
	s = syn.addFlag( "-l", "-list" );
	assert(s);
	
	s = syn.addFlag( "-st", "-sampleTime", MSyntax::kDouble );
	assert(s);
	
	syn.setObjectType( MSyntax::kStringObjects );
	
	return syn;
}
		
MStatus DelightProceduralCacheCommand::doIt( const MArgList &args )
{
	MArgParser parser( syntax(), args );
	MStatus s;
	
	if( parser.isFlagSet( "-a" ) )
	{
		MStringArray objectNames;
		s = parser.getObjects( objectNames );
		if( !s || objectNames.length()!=1 )
		{
			displayError( "DelightProceduralCacheCommand::doIt : unable to get object name argument." );
			return s;
		}
		
		MSelectionList sel;
		sel.add( objectNames[0] );
		MObject oDepNode;
		s = sel.getDependNode( 0, oDepNode );
		if( !s )
		{
			displayError( "DelightProceduralCacheCommand::doIt : unable to get dependency node for \"" + objectNames[0] + "\"." );
			return s;
		}
		MFnDagNode fnDagNode( oDepNode );
		IECoreMaya::ProceduralHolder *pHolder = dynamic_cast<IECoreMaya::ProceduralHolder *>( fnDagNode.userNode() );
		if( !pHolder )
		{
			displayError( "DelightProceduralCacheCommand::doIt : \"" + objectNames[0] + "\" is not a procedural holder node." );
			return MStatus::kFailure;
		}
		
		ProceduralMap::iterator pIt = g_procedurals.find( objectNames[0].asChar() );
		if( pIt!=g_procedurals.end() )
		{
			// we already got the procedural on the first sample, but we should expand the bounding box for this sample
			pIt->second.bound.extendBy( IECore::convert<Imath::Box3f>( fnDagNode.boundingBox() ) );
			return MStatus::kSuccess;
		}
		else
		{
			pHolder->setParameterisedValues();
			
			CachedProcedural cachedProcedural;
			cachedProcedural.procedural = pHolder->getProcedural( &cachedProcedural.className, &cachedProcedural.classVersion );
			if( !cachedProcedural.procedural )
			{
				displayError( "DelightProceduralCacheCommand::doIt : failed to get procedural from \"" + objectNames[0] + "\"." );
				return MStatus::kFailure;
			}
			cachedProcedural.bound = IECore::convert<Imath::Box3f>( fnDagNode.boundingBox() );
			IECore::ObjectPtr values = cachedProcedural.procedural->parameters()->getValue();
			if( !values )
			{
				displayError( "DelightProceduralCacheCommand::doIt : failed to get parameter values from \"" + objectNames[0] + "\"." );
				return MStatus::kFailure;
			}
			cachedProcedural.values = values->copy();				
			g_procedurals[objectNames[0].asChar()] = cachedProcedural;
		}

		return MStatus::kSuccess;
	}
	else if( parser.isFlagSet( "-l" ) )
	{
		MStringArray result;
		for( ProceduralMap::const_iterator it=g_procedurals.begin(); it!=g_procedurals.end(); it++ )
		{
			result.append( it->first.c_str() );
		}
		setResult( result );
		return MStatus::kSuccess;
	}
	else if( parser.isFlagSet( "-e" ) )
	{
		// get the object name
		MStringArray objectNames;
		s = parser.getObjects( objectNames );
		if( !s || objectNames.length()!=1 )
		{
			displayError( "DelightProceduralCacheCommand::doIt : unable to get object name argument." );
			return s;
		}
		
		// get the cached procedural
		ProceduralMap::const_iterator it = g_procedurals.find( objectNames[0].asChar() );
		if( it==g_procedurals.end() )
		{
			displayError( "DelightProceduralCacheCommand::doIt : unable to emit \"" + objectNames[0] + "\" as object has not been cached." );
			return MS::kFailure;
		}
		
		// and output it
		try
		{
			IECore::ObjectPtr currentValues = it->second.procedural->parameters()->getValue();			
			it->second.procedural->parameters()->setValue( it->second.values );			
			
			std::string pythonString;
			try 
			{
				// we first get an object referencing the serialise result and then make an extractor for it.
				// making the extractor directly from the return of the serialise call seems to result
				// in the python object dying before we properly extract the value, which results in corrupted
				// strings, and therefore malformed ribs.
				object serialisedResultObject = PythonCmd::globalContext()["IECore"].attr("ParameterParser")().attr("serialise")( it->second.procedural->parameters() );
				extract<std::string> serialisedResultExtractor( serialisedResultObject );
				
				std::string serialisedParameters = serialisedResultExtractor();
				pythonString = boost::str( boost::format( "IECoreRI.executeProcedural( \"%s\", %d, \"%s\" )" ) % it->second.className % it->second.classVersion % serialisedParameters );
			}
			catch( ... )
			{
				// Make sure we don't lose the 'current' values if we except.
				it->second.procedural->parameters()->setValue( currentValues );			
				displayError( "DelightProceduralCacheCommand::doIt : could not get parameters from \"" + objectNames[0] + "\"." );
				return MStatus::kFailure;
			}
			
			// Put the current values back.
			it->second.procedural->parameters()->setValue( currentValues );	
			
			if( it->second.bound.isEmpty() )
			{
				displayWarning( "DelightProceduralCacheCommand::doIt : not outputting procedural \"" + objectNames[0] + "\" because it has an empty bounding box." );
				return MS::kSuccess;
			}
			RtBound rtBound;
			IECore::convert( it->second.bound, rtBound );
			
			IECore::RendererPtr renderer = new IECoreRI::Renderer();
			IECore::AttributeBlock attributeBlock( renderer, 1 );
			
				it->second.procedural->render( renderer.get(), false, true, false, false );

				// tell 3delight we can't run multiple python procedurals concurrently
				int zero = 0;
				RiAttribute( "procedural", "integer reentrant", &zero, 0 );

				const char **data = (const char **)malloc( sizeof( char * ) * 2 );
				data[0] = STRINGIFY( IECORERI_RMANPROCEDURAL_NAME );
				data[1] = pythonString.c_str();
				RiProcedural( data, rtBound, RiProcDynamicLoad, RiProcFree );
			
		}
		catch( error_already_set )
		{
			PyErr_Print();
			displayError( "DelightProceduralCacheCommand::doIt : failed to output procedural for \"" + objectNames[0] + "\"." );
			return MStatus::kFailure;
		}
		catch( ... )
		{
			displayError( "DelightProceduralCacheCommand::doIt : failed to output procedural for \"" + objectNames[0] + "\"." );
			return MStatus::kFailure;
		}

		return MStatus::kSuccess;
	}
	else if( parser.isFlagSet( "-r" ) )
	{
		MStringArray objectNames;
		s = parser.getObjects( objectNames );
		if( !s || objectNames.length()!=1 )
		{
			displayError( "DelightProceduralCacheCommand::doIt : unable to get object name argument." );
			return s;
		}
		g_procedurals.erase( objectNames[0].asChar() );
		return MStatus::kSuccess;
	}
	else if( parser.isFlagSet( "-f" ) )
	{
		g_procedurals.clear();
		return MStatus::kSuccess;
	}
	
	displayError( "DelightProceduralCacheCommand::doIt : No suitable flag specified." );
	return MS::kFailure;
}
