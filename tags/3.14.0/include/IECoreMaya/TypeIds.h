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

#ifndef IECOREMAYA_TYPEIDS_H
#define IECOREMAYA_TYPEIDS_H

namespace IECoreMaya
{

enum TypeId
{
	
	FromMayaConverterTypeId = 109000,
	FromMayaObjectConverterTypeId = 109001,
	FromMayaPlugConverterTypeId = 109002,
	FromMayaMeshConverterTypeId = 109003,
	FromMayaCameraConverterTypeId = 109004,
	FromMayaGroupConverterTypeId = 109005,
	FromMayaNumericDataConverterTypeId = 109006,
	FromMayaNumericPlugConverterTypeId = 109007,
	FromMayaFluidConverterTypeId = 109008,
	FromMayaStringPlugConverterTypeId = 109009,
	FromMayaShapeConverterTypeId = 109010,
	FromMayaCurveConverterTypeId = 109011,
	FromMayaParticleConverterTypeId = 109012,
	FromMayaDagNodeConverterTypeId = 109013,
	ToMayaConverterTypeId = 109014,
	ToMayaObjectConverterTypeId = 109015,
	ToMayaNumericDataConverterTypeId = 109016,
	ToMayaMeshConverterTypeId = 109017,
	ToMayaArrayDataConverterTypeId = 109018,
	ToMayaPlugConverterTypeId = 109019,
	FromMayaPluginDataPlugConverterTypeId = 109020,	
	LastTypeId = 109999

};

} // namespace IECoreMaya

#endif // IECOREMAYA_TYPEIDS_H
