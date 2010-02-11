//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/static_assert.hpp"

#include "IECore/NumericParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"

using namespace std;
using namespace IECore;
using namespace boost;

template<class T>
Object::TypeDescription<NumericParameter<T> > NumericParameter<T>::g_typeDescription;

template<class T>
const unsigned int NumericParameter<T>::g_ioVersion = 1;

/////////////////////////////////////////////////////////////////////////////////////
// constructor stuff
/////////////////////////////////////////////////////////////////////////////////////

template<typename T>
static Parameter::PresetsContainer convertPresets( const typename NumericParameter<T>::PresetsContainer p )
{
	Parameter::PresetsContainer result; result.reserve( p.size() );
	for( typename NumericParameter<T>::PresetsContainer::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.push_back( typename Parameter::PresetsContainer::value_type( it->first, new TypedData<T>( it->second ) ) );
	}
	return result;
}

template<typename T>
NumericParameter<T>::NumericParameter()
	:	m_min( Imath::limits<T>::min() ), m_max( Imath::limits<T>::max() )
{
}

template<typename T>
NumericParameter<T>::NumericParameter( const std::string &name, const std::string &description, T defaultValue,
	T minValue, T maxValue, const PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, new ObjectType( defaultValue ), convertPresets<T>( presets ), presetsOnly, userData ), m_min( minValue ), m_max( maxValue )
{
	if ( defaultValue < minValue || defaultValue > maxValue )
	{
		throw InvalidArgumentException( "NumericParameter default value outside specified min/max range" );
	}
}

template<typename T>
NumericParameter<T>::NumericParameter( const std::string &name, const std::string &description, T defaultValue,
	const PresetsContainer &presets, ConstCompoundObjectPtr userData )
	: Parameter( name, description, new ObjectType( defaultValue ), convertPresets<T>( presets ), true, userData ), m_min( Imath::limits<T>::min() ), m_max( Imath::limits<T>::max() )
{
}

/////////////////////////////////////////////////////////////////////////////////////
// object stuff
/////////////////////////////////////////////////////////////////////////////////////

template <class T>
typename NumericParameter<T>::Ptr NumericParameter<T>::copy() const
{
	return boost::static_pointer_cast<NumericParameter<T> >( copy() );
}

template<class T>
void NumericParameter<T>::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Parameter::copyFrom( other, context );
	const NumericParameter<T> *tOther = static_cast<const NumericParameter<T> *>( other.get() );
	m_min = tOther->m_min;
	m_max = tOther->m_max;
}

template<class T>
void NumericParameter<T>::save( SaveContext *context ) const
{
	Parameter::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), g_ioVersion );
	container->write( "min", m_min );
	container->write( "max", m_max );
}

template<class T>
void NumericParameter<T>::load( LoadContextPtr context )
{
	Parameter::load( context );
	unsigned int v = g_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	container->read( "min", m_min );
	container->read( "max", m_max );
}

template<class T>
bool NumericParameter<T>::isEqualTo( ConstObjectPtr other ) const
{
	if( !Parameter::isEqualTo( other ) )
	{
		return false;
	}
	const NumericParameter<T> *tOther = static_cast<const NumericParameter<T> *>( other.get() );
	return m_min==tOther->m_min && m_max==tOther->m_max;
}

template<class T>
void NumericParameter<T>::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Parameter::memoryUsage( a );
	a.accumulate( 2 * sizeof( T ) );
}

/////////////////////////////////////////////////////////////////////////////////////
// other stuff
/////////////////////////////////////////////////////////////////////////////////////

template<typename T>
bool NumericParameter<T>::hasMinValue() const
{
	return m_min != Imath::limits<T>::min();
}

template<typename T>
T NumericParameter<T>::minValue() const
{
	return m_min;
}

template<typename T>
bool NumericParameter<T>::hasMaxValue() const
{
	return m_max != Imath::limits<T>::max();
}

template<typename T>
T NumericParameter<T>::maxValue() const
{
	return m_max;
}

template<typename T>
T NumericParameter<T>::numericDefaultValue() const
{
	const TypedData<T> *v = static_cast<const TypedData<T> *>( defaultValue() );
	return v->readable();
}

template<typename T>
T NumericParameter<T>::getNumericValue() const
{
	const TypedData<T> *v = runTimeCast<const TypedData<T> >( getValidatedValue() );
	if (!v)
	{
		throw Exception( string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"" );
	}
	return v->readable();
}

template<typename T>
void NumericParameter<T>::setNumericValue( T value )
{
	setValue( new TypedData<T>( value ) );
}

template<typename T>
bool NumericParameter<T>::valueValid( const Object *value, std::string *reason ) const
{
	if( !Parameter::valueValid( value, reason ) )
	{
		return false;
	}
	const ObjectType *tValue = runTimeCast<const ObjectType>( value );
	if( !tValue )
	{
		if( reason )
		{
			*reason = string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"";
		}
		return false;
	}
	if( tValue->readable() < m_min || tValue->readable() > m_max )
	{
		if( reason )
		{
			*reason = "Value is not in range.";
		}
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// specialisation and template instantiation
/////////////////////////////////////////////////////////////////////////////////////


#define IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( T, TNAME ) \
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TNAME ## TypeId ); \
	\
	template class NumericParameter<T>;

namespace IECore
{
IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( int, IntParameter )
IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( float, FloatParameter )
IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( double, DoubleParameter )
}
