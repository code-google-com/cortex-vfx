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

#include <cassert>

#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"

#include "Convert.h"

namespace IECore
{

template<>
UT_Vector3 convert( const Imath::V3f &from )
{
	return UT_Vector3( from.x, from.y, from.z );
}

template<>
Imath::V3f convert( const UT_Vector3 &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
UT_BoundingBox convert( const Imath::Box3f &from )
{
	if( from.isEmpty() )
	{
		return UT_BoundingBox();
	}
	return UT_BoundingBox( convert<UT_Vector3>( from.min ), convert<UT_Vector3>( from.max ) );
}

template<>
Imath::Box3f convert( const UT_BoundingBox &from )
{
	return Imath::Box3f( convert<Imath::V3f>( from.minvec() ), convert<Imath::V3f>( from.maxvec() ) );
}

} // namespace IECore
