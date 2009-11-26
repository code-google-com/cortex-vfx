//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathFun.h"

#include "IECore/CurvesPrimitiveEvaluator.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/Exception.h"
#include "IECore/FastFloat.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( CurvesPrimitiveEvaluator );

//////////////////////////////////////////////////////////////////////////
// Implementation of Result
//////////////////////////////////////////////////////////////////////////

CurvesPrimitiveEvaluator::Result::Result()
{
}

template<class T>
T CurvesPrimitiveEvaluator::Result::primVar( const PrimitiveVariable &primVar, const float *coefficients ) const
{
	switch( primVar.interpolation )
	{
		case PrimitiveVariable::Constant :
			{
				const TypedData<T> *d = static_cast<TypedData<T> *>( primVar.data.get() );
				return d->readable();
			}
			break;
		case PrimitiveVariable::Uniform :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				return d[m_curveIndex];
			}
		case PrimitiveVariable::Vertex :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				
				if ( m_linear )
				{
					return	(T)( coefficients[0] * d[m_vertexDataIndices[0]] +
						coefficients[1] * d[m_vertexDataIndices[1]] );
				}
				else
				{
					return	(T)( coefficients[0] * d[m_vertexDataIndices[0]] +
						coefficients[1] * d[m_vertexDataIndices[1]] +
						coefficients[2] * d[m_vertexDataIndices[2]] +
						coefficients[3] * d[m_vertexDataIndices[3]] );
				}
			}
		case PrimitiveVariable::Varying :
		case PrimitiveVariable::FaceVarying :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				return lerp( d[m_varyingDataIndices[0]], d[m_varyingDataIndices[1]], m_segmentV );
			}
		default :
			throw InvalidArgumentException( "PrimitiveVariable has invalid interpolation" );
	}
}

Imath::V3f CurvesPrimitiveEvaluator::Result::point() const
{
	return primVar<V3f>( m_p, m_coefficients );
}

Imath::V3f CurvesPrimitiveEvaluator::Result::normal() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V2f CurvesPrimitiveEvaluator::Result::uv() const
{
	return V2f( 0, m_v );
}

Imath::V3f CurvesPrimitiveEvaluator::Result::uTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V3f CurvesPrimitiveEvaluator::Result::vTangent() const
{
	return primVar<V3f>( m_p, m_derivativeCoefficients );
}

unsigned CurvesPrimitiveEvaluator::Result::curveIndex() const
{
	return m_curveIndex;
}

Imath::V3f CurvesPrimitiveEvaluator::Result::vectorPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<V3f>( pv, m_coefficients );
}

float CurvesPrimitiveEvaluator::Result::floatPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<float>( pv, m_coefficients );
}

int CurvesPrimitiveEvaluator::Result::intPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<int>( pv, m_coefficients );
}

const std::string &CurvesPrimitiveEvaluator::Result::stringPrimVar( const PrimitiveVariable &pv ) const
{
	switch( pv.interpolation )
	{
		case PrimitiveVariable::Constant :
			{
				const TypedData<string> *d = static_cast<TypedData<string> *>( pv.data.get() );
				return d->readable();
			}
			break;
		case PrimitiveVariable::Uniform :
			{
				const vector<string> &d = static_cast<TypedData<vector<string> > *>( pv.data.get() )->readable();
				return d[m_curveIndex];
			}
		default :
			{
				throw InvalidArgumentException( "Can only evaluate string PrimitiveVariables with Constant or Uniform interpolation." );
			}
	}
}

Imath::Color3f CurvesPrimitiveEvaluator::Result::colorPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<Color3f>( pv, m_coefficients );
}

half CurvesPrimitiveEvaluator::Result::halfPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<half>( pv, m_coefficients );
}

void CurvesPrimitiveEvaluator::Result::init( unsigned curveIndex, float v, const CurvesPrimitiveEvaluator *evaluator )
{
	m_curveIndex = curveIndex;
	m_v = v;

	unsigned numSegments = evaluator->m_curvesPrimitive->numSegments( m_curveIndex );
	float vv = v * numSegments;
	unsigned segment = min( (unsigned)fastFloatFloor( vv ), numSegments - 1 );
	m_segmentV = vv - segment;
	
	const CubicBasisf &basis = evaluator->m_curvesPrimitive->basis();
	basis.coefficients( m_segmentV, m_coefficients );
	basis.derivativeCoefficients( m_segmentV, m_derivativeCoefficients );
	
	if( evaluator->m_curvesPrimitive->periodic() )
	{
		int numVertices = evaluator->m_verticesPerCurve[curveIndex];
		unsigned o = evaluator->m_vertexDataOffsets[m_curveIndex];
		unsigned i = segment * basis.step;
		m_vertexDataIndices[0] = o + i;
		m_vertexDataIndices[1] = o + ( ( i + 1 ) % numVertices );
		m_vertexDataIndices[2] = o + ( ( i + 2 ) % numVertices );
		m_vertexDataIndices[3] = o + ( ( i + 3 ) % numVertices );
		
		m_varyingDataIndices[0] = evaluator->m_varyingDataOffsets[m_curveIndex] + segment;
		m_varyingDataIndices[1] = evaluator->m_varyingDataOffsets[m_curveIndex] + ( segment % numSegments );
	}
	else
	{
		m_vertexDataIndices[0] = evaluator->m_vertexDataOffsets[m_curveIndex] + segment * basis.step;
		m_vertexDataIndices[1] = m_vertexDataIndices[0] + 1;
		m_vertexDataIndices[2] = m_vertexDataIndices[1] + 1;
		m_vertexDataIndices[3] = m_vertexDataIndices[2] + 1;
	
		m_varyingDataIndices[0] = evaluator->m_varyingDataOffsets[m_curveIndex] + segment;
		m_varyingDataIndices[1] = m_varyingDataIndices[0] + 1;
	}
}
				
//////////////////////////////////////////////////////////////////////////
// Implementation of Evaluator
//////////////////////////////////////////////////////////////////////////

CurvesPrimitiveEvaluator::CurvesPrimitiveEvaluator( ConstCurvesPrimitivePtr curves )
	:	m_curvesPrimitive( curves->copy() ), m_verticesPerCurve( m_curvesPrimitive->verticesPerCurve()->readable() )
{
	m_vertexDataOffsets.reserve( m_verticesPerCurve.size() );
	m_varyingDataOffsets.reserve( m_verticesPerCurve.size() );
	int vertexDataOffset = 0;
	int varyingDataOffset = 0;
	for( unsigned i=0; i<m_verticesPerCurve.size(); i++ )
	{
		m_vertexDataOffsets.push_back( vertexDataOffset );
		vertexDataOffset += m_verticesPerCurve[i];
		
		m_varyingDataOffsets.push_back( varyingDataOffset );
		varyingDataOffset += m_curvesPrimitive->variableSize( PrimitiveVariable::Varying, i );
	}
	
	PrimitiveVariableMap::iterator pIt = m_curvesPrimitive->variables.find( "P" );
	if( pIt==m_curvesPrimitive->variables.end() )
	{
		throw InvalidArgumentException( "No PrimitiveVariable named P on CurvesPrimitive." );
	}
	m_p = pIt->second;
}

CurvesPrimitiveEvaluator::~CurvesPrimitiveEvaluator()
{
}

ConstPrimitivePtr CurvesPrimitiveEvaluator::primitive() const
{
	return m_curvesPrimitive;
}

PrimitiveEvaluator::ResultPtr CurvesPrimitiveEvaluator::createResult() const
{
	Result *result = new Result;
	result->m_p = m_p;
	result->m_linear = m_curvesPrimitive->basis() == CubicBasisf::linear();
	return result;
}

void CurvesPrimitiveEvaluator::validateResult( const PrimitiveEvaluator::ResultPtr &result ) const
{
	if( ! boost::dynamic_pointer_cast<CurvesPrimitiveEvaluator::Result>( result ) )
	{
		throw InvalidArgumentException( "CurvesPrimitiveEvaluator: Invalid result type" );
	}
}

float CurvesPrimitiveEvaluator::surfaceArea() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

float CurvesPrimitiveEvaluator::volume() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V3f CurvesPrimitiveEvaluator::centerOfGravity() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

bool CurvesPrimitiveEvaluator::closestPoint( const Imath::V3f &p, const PrimitiveEvaluator::ResultPtr &result ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

bool CurvesPrimitiveEvaluator::pointAtUV( const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const
{
	return pointAtV( 0, uv[1], result );
}

bool CurvesPrimitiveEvaluator::intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
	const PrimitiveEvaluator::ResultPtr &result, float maxDistance ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

int CurvesPrimitiveEvaluator::intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
	std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

bool CurvesPrimitiveEvaluator::pointAtV( unsigned curveIndex, float v, const PrimitiveEvaluator::ResultPtr &result ) const
{
	if( curveIndex >= m_verticesPerCurve.size() || v < 0.0f || v > 1.0f )
	{
		return false;
	}
	Result *typedResult = static_cast<Result *>( result.get() );
	typedResult->init( curveIndex, v, this );
	return true;
}

float CurvesPrimitiveEvaluator::curveLength( unsigned curveIndex, float vStart, float vEnd ) const
{
	if( curveIndex >= m_verticesPerCurve.size() || vStart >= vEnd || vStart < 0.0f || vStart > 1.0f || vEnd < 0.0f || vEnd > 1.0f )
	{
		return 0.0f;
	}
	
	float samples = m_curvesPrimitive->numSegments( curveIndex ) * 10.0f;
	float vStep = ( vEnd - vStart ) / samples;
	float length = 0.0f;
	Imath::V3f current( 0, 0, 0 );
	
	Result typedResult;
	typedResult.m_p = m_p;
	typedResult.m_linear = m_curvesPrimitive->basis() == CubicBasisf::linear();
	typedResult.init( curveIndex, vStart, this );
	Imath::V3f previous = typedResult.point();
	
	if ( typedResult.m_linear )
	{
		float firstV = 0.0f;
		float lastV = 1.0f;
		vStep = 1.0f / m_curvesPrimitive->numSegments( curveIndex );
		
		while ( firstV <= vStart )
		{
			firstV += vStep;
		}
		while ( lastV >= vEnd )
		{
			lastV -= vStep;
		}
		
		for ( float v = firstV; v <= vEnd; v = ( v == lastV ) ? vEnd : v + vStep )
		{
			typedResult.init( curveIndex, v, this );
			
			current = typedResult.point();
			length += ( current - previous ).length();
			previous = current;
		}
		
		return length;
	}
	
	for ( float v = vStart + vStep; v <= vEnd; v += vStep )
	{
		typedResult.init( curveIndex, v, this );
		
		current = typedResult.point();
		length += ( current - previous ).length();
		previous = current;
	}
 	
	return length;
}
