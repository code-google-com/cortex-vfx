//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include <algorithm>

#include "boost/bind.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"

#include "IECore/CompoundParameter.h"
#include "IECore/NullObject.h"
#include "IECore/Exception.h"

using namespace std;
using namespace IECore;
using namespace boost;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( CompoundParameter );
const unsigned int CompoundParameter::g_ioVersion = 1;

CompoundParameter::CompoundParameter( const std::string &name, const std::string &description, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, new CompoundObject, PresetsContainer(), false, userData )
{
}

// We could just not implement these functions and instead keep the default value and presets up
// to date in addParameter(). But I'd anticipate us allowing the modification of the default and
// the presets in future versions, in which case we'd need an implementation like that below, so
// i'm going with that.
ConstObjectPtr CompoundParameter::defaultValue() const
{
	ConstCompoundObjectPtr constValue = static_pointer_cast<const CompoundObject>( Parameter::defaultValue() );
	// naughty? not really i reckon - it results in the right semantics to the outside
	// observer.
	CompoundObjectPtr value = const_pointer_cast<CompoundObject>( constValue );
	value->members().clear();
	CompoundObject::ObjectMap &m = value->members();
	for( ParameterMap::const_iterator it=m_namesToParameters.begin(); it!=m_namesToParameters.end(); it++ )
	{
		m[it->first] = const_pointer_cast<Object>( it->second->defaultValue() );
	}
	return value;
}

const Parameter::PresetsContainer &CompoundParameter::presets() const
{
	// naughty? nah! it gives the right semantics to an outside observer
	PresetsContainer &pr = const_cast<PresetsContainer &>( Parameter::presets() );
	pr.clear();
	if( !m_namesToParameters.size() )
	{
		return pr;
	}

	// get a references for each child preset map.
	// we only want to call presets() once for
	// each child as the map returned may change between calls.
	vector<const PresetsContainer *> childPresets;
	for( size_t i=0; i<m_parameters.size(); i++ )
	{
		childPresets.push_back( &(m_parameters[i]->presets()) );
	}

	// find the intersection of all the child preset names
	set<string> names;
	for( PresetsContainer::const_iterator it=childPresets[0]->begin(); it!=childPresets[0]->end(); it++ )
	{
		bool ok = true;
		for( size_t i=1; i<m_parameters.size(); i++ )
		{
			if( find_if( childPresets[i]->begin(), childPresets[i]->end(), bind( &Preset::first, _1 )==it->first )==childPresets[i]->end() )
			{
				ok = false;
				break;
			}
		}
		if( ok )
		{
			names.insert( it->first );
		}
	}

	for( set<string>::const_iterator nIt=names.begin(); nIt!=names.end(); nIt++ )
	{
		CompoundObjectPtr o = new CompoundObject;
		for( size_t i=0; i<m_parameters.size(); i++ )
		{
			o->members()[m_parameters[i]->name()] = find_if( childPresets[i]->begin(), childPresets[i]->end(), bind( &Preset::first, _1 )==*nIt )->second;
		}
		pr.push_back( Preset( *nIt, o ) );
	}

	return pr;
}

bool CompoundParameter::presetsOnly() const
{
	for( ParameterVector::const_iterator it=m_parameters.begin(); it!=m_parameters.end(); it++ )
	{
		if( !(*it)->presetsOnly() )
		{
			return false;
		}
	}
	return true;
}

void CompoundParameter::setValue( ObjectPtr value )
{
	Parameter::setValue( value );
	CompoundObjectPtr tValue = runTimeCast<CompoundObject>( value );
	if( tValue )
	{
		ParameterMap::iterator it;
		for( it=m_namesToParameters.begin(); it!=m_namesToParameters.end(); it++ )
		{
			CompoundObject::ObjectMap::iterator oIt = tValue->members().find( it->first );
			if( oIt!=tValue->members().end() )
			{
				it->second->setValue( oIt->second );
			}
			else
			{
				it->second->setValue( new NullObject );
			}
		}
	}
}

ObjectPtr CompoundParameter::getValue()
{
	ObjectPtr value = Parameter::getValue();
	CompoundObjectPtr tValue = runTimeCast<CompoundObject>( value );
	if( !tValue )
	{
		return value;
	}

	CompoundObject::ObjectMap &m = tValue->members();
	for( ParameterMap::const_iterator it=m_namesToParameters.begin(); it!=m_namesToParameters.end(); it++ )
	{
		m[it->first] = it->second->getValue();
	}
	return tValue;
}

ConstObjectPtr CompoundParameter::getValue() const
{
	// naughty?
	// we cast away constness from this to allow us to update the compound value
	// before returning it, which actually results in the correct semantics at the
	// interface level.
	CompoundParameter *mutableThis = const_cast<CompoundParameter *>( this );
	ObjectPtr value = mutableThis->getValue();
	return value;
}

bool CompoundParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	if( !Parameter::valueValid( value, reason ) )
	{
		return false;
	}
	ConstCompoundObjectPtr tValue = runTimeCast<const CompoundObject>( value );
	if( !tValue )
	{
		if( reason )
		{
			*reason = boost::str( boost::format( "Value is of type \"%s\" and not of type \"CompoundObject\"." ) % value->typeName() );
		}
		return false;
	}
	if( tValue->members().size()!=m_parameters.size() )
	{
		if( reason )
		{
			*reason = "Number of CompoundObject members doesn't match number of parameters.";
		}
		return false;
	}
	CompoundObject::ObjectMap::const_iterator it;
	for( it=tValue->members().begin(); it!=tValue->members().end(); it++ )
	{
		ParameterMap::const_iterator pIt = m_namesToParameters.find( it->first );
		if( pIt==m_namesToParameters.end() )
		{
			if( reason )
			{
				*reason = "CompoundObject member names do not match parameter names.";
			}
			return false;
		}
		else
		{
			/// \todo Prepend the child parameter name to the message to make it more useful.
			if( !pIt->second->valueValid( it->second, reason ) )
			{
				return false;
			}
		}
	}
	return true;
}

void CompoundParameter::addParameter( ParameterPtr parameter )
{
	if( m_namesToParameters.find( parameter->internedName() )!=m_namesToParameters.end() )
	{
		throw Exception( boost::str( boost::format( "Child parameter named \"%s\" already exists." ) % parameter->name() ) );
	}
	m_namesToParameters.insert( ParameterMap::value_type( parameter->internedName(), parameter ) );
	m_parameters.push_back( parameter );
}

void CompoundParameter::insertParameter( ParameterPtr parameter, ConstParameterPtr other )
{
	if( m_namesToParameters.find( parameter->internedName() )!=m_namesToParameters.end() )
	{
		throw Exception( boost::str( boost::format( "Child parameter named \"%s\" already exists." ) % parameter->name() ) );
	}
	ParameterVector::iterator it = find( m_parameters.begin(), m_parameters.end(), other );
	if( it==m_parameters.end() )
	{
		throw Exception( "Parameter to insert before is not a child." );
	}
	m_namesToParameters.insert( ParameterMap::value_type( parameter->internedName(), parameter ) );
	m_parameters.insert( it, parameter );
}

void CompoundParameter::removeParameter( ParameterPtr parameter )
{
	ParameterVector::iterator it = find( m_parameters.begin(), m_parameters.end(), parameter );
	if( it==m_parameters.end() )
	{
		throw Exception( "Parameter to remove doesn't exist" );
	}
	m_parameters.erase( it );
	m_namesToParameters.erase( parameter->internedName() );

	ObjectPtr value = Parameter::getValue();
	CompoundObjectPtr tValue = runTimeCast<CompoundObject>( value );
	if( tValue )
	{
		CompoundObject::ObjectMap::iterator oIt = tValue->members().find( parameter->name() );
		if( oIt!=tValue->members().end() )
		{
			tValue->members().erase( oIt );
		}
	}

}

void CompoundParameter::removeParameter( const std::string &name )
{
	ParameterMap::iterator it = m_namesToParameters.find( name );
	if( it==m_namesToParameters.end() )
	{
		throw Exception( string("Parameter ") + name + " doesn't exist" );
	}
	removeParameter( it->second );
}

void CompoundParameter::clearParameters()
{
	while( m_parameters.size() )
	{
		removeParameter( m_parameters[0] );
	}
}

const CompoundParameter::ParameterMap &CompoundParameter::parameters() const
{
	return m_namesToParameters;
}

const CompoundParameter::ParameterVector &CompoundParameter::orderedParameters() const
{
	return m_parameters;
}

void CompoundParameter::setParameterValue( const std::string &name, ObjectPtr value )
{
	ParameterPtr p = parameter<Parameter>( name );
	if( !p )
	{
		throw Exception( string("Parameter ") + name + " doesn't exist" );
	}
	p->setValue( value );
}

void CompoundParameter::setValidatedParameterValue( const std::string &name, ObjectPtr value )
{
	ParameterPtr p = parameter<Parameter>( name );
	if( !p )
	{
		throw Exception( string("Parameter ") + name + " doesn't exist" );
	}
	p->setValidatedValue( value );
}

ObjectPtr CompoundParameter::getParameterValue( const std::string &name )
{
	ParameterPtr p = parameter<Parameter>( name );
	if( !p )
	{
		throw Exception( string("Parameter ") + name + " doesn't exist" );
	}
	return p->getValue();
}

ObjectPtr CompoundParameter::getValidatedParameterValue( const std::string &name )
{
	ParameterPtr p = parameter<Parameter>( name );
	if( !p )
	{
		throw Exception( string("Parameter ") + name + " doesn't exist" );
	}
	return p->getValidatedValue();
}

bool CompoundParameter::parameterPath( ConstParameterPtr child, std::vector<std::string> &path ) const
{
	for( ParameterVector::const_iterator it=m_parameters.begin(); it!=m_parameters.end(); it++ )
	{
		if( child==*it )
		{
			path.insert( path.begin(), child->name() );
			return true;
		}
		else
		{
			ConstCompoundParameterPtr c = runTimeCast<const CompoundParameter>( *it );
			if( c )
			{
				if( c->parameterPath( child, path ) )
				{
					path.insert( path.begin(), c->name() );
					return true;
				}
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Object implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CompoundParameter::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Parameter::copyFrom( other, context );
	const CompoundParameter *tOther = static_cast<const CompoundParameter *>( other.get() );

	m_namesToParameters.clear();
	m_parameters.clear();
	for( ParameterVector::const_iterator it=tOther->m_parameters.begin(); it!=tOther->m_parameters.end(); it++ )
	{
		addParameter( (*it)->copy() );
	}
}

void CompoundParameter::save( SaveContext *context ) const
{
	Parameter::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), g_ioVersion );

	container->mkdir( "parameters" );
	container->chdir( "parameters" );
		unsigned i = 0;
		for( ParameterVector::const_iterator it=m_parameters.begin(); it!=m_parameters.end(); it++, i++ )
		{
			string name = str( boost::format( "%d" ) % i );
			context->save( *it, container, name );
		}
	container->chdir( ".." );
}

void CompoundParameter::load( LoadContextPtr context )
{
	Parameter::load( context );
	unsigned int v = g_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );

	m_namesToParameters.clear();
	m_parameters.clear();
	container->chdir( "parameters" );
		IndexedIO::EntryList l = container->ls();
		m_parameters.resize( l.size() );
		for( IndexedIO::EntryList::const_iterator it=l.begin(); it!=l.end(); it++ )
		{
			ParameterPtr parameter = context->load<Parameter>( container, it->id() );
			size_t i = boost::lexical_cast<size_t>( it->id() );
			m_namesToParameters.insert( ParameterMap::value_type( parameter->internedName(), parameter ) );
			m_parameters[i] = parameter;
		}
	container->chdir( ".." );
}

bool CompoundParameter::isEqualTo( ConstObjectPtr other ) const
{
	if( !Parameter::isEqualTo( other ) )
	{
		return false;
	}


	const CompoundParameter *tOther = static_cast<const CompoundParameter *>( other.get() );
	if( tOther->m_parameters.size()!=m_parameters.size() )
	{
		return false;
	}

	for( unsigned i=0; i<m_parameters.size(); i++ )
	{
		if( !m_parameters[i]->isEqualTo( tOther->m_parameters[i] ) )
		{
			return false;
		}
	}

	return true;
}

void CompoundParameter::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Parameter::memoryUsage( a );
	a.accumulate( m_parameters.capacity() * sizeof( ParameterVector::value_type ) );
	for( ParameterVector::const_iterator it=m_parameters.begin(); it!=m_parameters.end(); it++ )
	{
		a.accumulate( *it );
	}
	
	a.accumulate( m_namesToParameters.size() * sizeof( ParameterMap::value_type ) );
}
