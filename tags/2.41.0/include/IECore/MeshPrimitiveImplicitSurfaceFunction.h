//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_MESHPRIMITIVEIMPLICITSURFACEFUNCTION_H
#define IE_CORE_MESHPRIMITIVEIMPLICITSURFACEFUNCTION_H

#include <map>
#include <set>

#include "OpenEXR/ImathVec.h"

#include "IECore/MeshPrimitive.h"
#include "IECore/PrimitiveImplicitSurfaceFunction.h"

namespace IECore
{

/// A model of ImplicitSurfaceFunction for creating a signed distance field with respect to a MeshPrimitive.
/// \deprecated In future versions PrimitiveImplicitSurfaceFunction will be able to acheive the same functionality as this class, as it will defer all work to the PrimitiveEvaluator itself
class MeshPrimitiveImplicitSurfaceFunction : public PrimitiveImplicitSurfaceFunction
{
        public:
                typedef boost::intrusive_ptr<MeshPrimitiveImplicitSurfaceFunction> Ptr;
                typedef boost::intrusive_ptr<const MeshPrimitiveImplicitSurfaceFunction> ConstPtr;
		
		MeshPrimitiveImplicitSurfaceFunction( MeshPrimitivePtr mesh );
		
		virtual ~MeshPrimitiveImplicitSurfaceFunction();
                
		// Retrieve the signed distance from the mesh at the given point              
                Value operator()( const Point &p );
		
		// Retrieve the signed distance from the mesh at the given point		
		virtual Value getValue( const Point &p );

		
	protected:	
	
		typedef int VertexIndex;
		typedef int TriangleIndex;
		typedef std::pair<VertexIndex, VertexIndex> Edge;		
		
		typedef std::map< Edge, Imath::V3f > EdgeAverageNormals;		
		EdgeAverageNormals m_edgeAverageNormals;
	
		V3fVectorDataPtr m_vertexAngleWeightedNormals;
		
		ConstV3fVectorDataPtr m_P;				
};

IE_CORE_DECLAREPTR( MeshPrimitiveImplicitSurfaceFunction );

}

#endif // IE_CORE_MESHPRIMITIVEIMPLICITSURFACEFUNCTION_H
