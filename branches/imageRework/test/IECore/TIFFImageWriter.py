##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import unittest
import glob
import sys
import os
from IECore import *

class TestTIFFImageWriter(unittest.TestCase):

	def __makeFloatImage( self, dataWindow, displayWindow, withAlpha = False, dataType = FloatVectorData ) :
	
		img = ImagePrimitive( dataWindow, displayWindow )
		
		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1
		
		area = w * h
		R = dataType( area )
		G = dataType( area )		
		B = dataType( area )
		
		if withAlpha:
			A = dataType( area )
		
		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :
			
				R[offset] = float(x) / (w - 1)				
				G[offset] = float(y) / (h - 1)
				B[offset] = 0.0
				if withAlpha:
					A[offset] = 0.5
				
				offset = offset + 1				
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )		
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )
		
		if withAlpha:
			img["A"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, A )
		
		return img
		
	def __makeIntImage( self, dataWindow, displayWindow, dataType = UIntVectorData, maxInt = 2**32-1 ) :
	
		img = ImagePrimitive( dataWindow, displayWindow )
		
		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1
		
		area = w * h
		R = dataType( area )
		G = dataType( area )		
		B = dataType( area )
		
		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :
			
				R[offset] = int( maxInt * float(x) / (w - 1) )
				G[offset] = int( maxInt * float(y) / (h - 1) )
				B[offset] = 0
				
				offset = offset + 1				
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )		
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )
		
		return img	
		
	def __makeGreyscaleImage( self, dataWindow, displayWindow ) :
	
		img = ImagePrimitive( dataWindow, displayWindow )
		
		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1
		
		area = w * h
		Y = FloatVectorData( area )
		
		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :
			
				Y[offset] = float(x) / (w - 1)	* float(y) / (h - 1)			
				
				offset = offset + 1				
		
		img["Y"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, Y )
		
		return img
		
	def __makeComplexImage( self, dataWindow, displayWindow ) :
	
		img = ImagePrimitive( dataWindow, displayWindow )
		
		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1
		
		area = w * h
		R = FloatVectorData( area )
		G = HalfVectorData( area )		
		B = DoubleVectorData( area )
		
		A = UIntVectorData( area )
		
		Data1 = UShortVectorData( area )
		Data2 = UCharVectorData( area )
		
		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :
			
				R[offset] = float(x) / (w - 1)				
				G[offset] = float(y) / (h - 1)
				B[offset] = 0.0
				A[offset] = 1
				
				Data1[offset] = 8192
				Data2[offset] = 128
				
				offset = offset + 1				
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )		
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )		
		img["A"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, A )
		img["Data1"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, Data1 )
		img["Data2"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, Data2 )
		
		return img		

	def testConstruction( self ):
		
		img = ImagePrimitive()			
		w = Writer.create( img, "test/IECore/data/tiff/output.tif" )
		self.assertEqual( type(w), TIFFImageWriter )
		
	def testType( self ) :	
	
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 199, 99 )
		)
		
		dataWindow = displayWindow	
		
		img = self.__makeFloatImage( dataWindow, displayWindow )
		
		for b in [ 8, 16, 32 ]:
		
			w = Writer.create( img, "test/IECore/data/tiff/output" + str(b) + ".tif" )
			self.assertEqual( type(w), TIFFImageWriter )
		
			w.parameters().bitdepth = b
			w.write()
		
			#self.assert_( os.path.exists( "test/IECore/data/tiff/output.tif" ) )
			

	def testWrite( self ) :	
		
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)
		
		dataWindow = displayWindow
		
		for dataType in [ FloatVectorData, HalfVectorData, DoubleVectorData ] :
		
			img = self.__makeFloatImage( dataWindow, displayWindow, dataType = dataType )
		
			w = Writer.create( img, "test/IECore/data/tiff/output" + dataType.staticTypeName() + ".tif" )
			self.assertEqual( type(w), TIFFImageWriter )
		
			w.write()
		
#			self.assert_( os.path.exists( "test/IECore/data/tiff/output.tif" ) )
			
#			if dataType == FloatVectorData:
			
#				self.assertEqual( os.path.getsize( "test/IECore/data/tiff/output.tif" ), 28104 )
				
		for dataType in [ ( UIntVectorData, 2**32-1), (UCharVectorData, 2**8-1 ),  (UShortVectorData, 2**16-1 ) ] :
		
			img = self.__makeIntImage( dataWindow, displayWindow, dataType = dataType[0], maxInt = dataType[1] )
		
			w = Writer.create( img, "test/IECore/data/tiff/output" + dataType[0].staticTypeName() + ".tif" )
			self.assertEqual( type(w), TIFFImageWriter )
		
			w.write()
		
			#self.assert_( os.path.exists( "test/IECore/data/tiff/output.tif" ) )
			
		img = self.__makeFloatImage( dataWindow, displayWindow, withAlpha = True )
		
		w = Writer.create( img, "test/IECore/data/tiff/outputAlpha.tif" )
		self.assertEqual( type(w), TIFFImageWriter )
		
		w.write()			
		
		img = self.__makeComplexImage( dataWindow, displayWindow )
		
		w = Writer.create( img, "test/IECore/data/tiff/outputComplex.tif" )
		self.assertEqual( type(w), TIFFImageWriter )
		
		w.write()
		
	def testGreyscaleWrite( self ) :
	
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 199, 99 )
		)
		
		dataWindow = displayWindow	
		
		img = self.__makeGreyscaleImage( dataWindow, displayWindow )
		
		w = Writer.create( img, "test/IECore/data/tiff/output.tif" )
		self.assertEqual( type(w), TIFFImageWriter )
		
		w.write()
		
		self.assert_( os.path.exists( "test/IECore/data/tiff/output.tif" ) )
		
		r = Reader.create( "test/IECore/data/tiff/output.tif" )
		img2 = r.read()
		
		channelNames = r.channelNames()
		self.assertEqual( len(channelNames), 1 )
		
	def testWriteIncomplete( self ) :
	
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)
		
		dataWindow = displayWindow
		
		img = self.__makeFloatImage( dataWindow, displayWindow )
		
		# We don't have enough data to fill this dataWindow
		img.dataWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 199, 199 )
		)
		
		self.failIf( img.arePrimitiveVariablesValid() )
		
		w = Writer.create( img, "test/IECore/data/tiff/output.tif" )
		self.assertEqual( type(w), TIFFImageWriter )
		
		# \todo The writer should give us some sort of error, but doesn't. It crashes.
		self.failIf( True )		
		w.write()
		
		
		self.failIf( os.path.exists( "test/IECore/data/tiff/output.tif" ) )
		
	def testErrors( self ) :
		
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)
		
		dataWindow = displayWindow
		
		
		# Try and write an image with the "R" channel of an unsupported type
		img = self.__makeFloatImage( dataWindow, displayWindow )	
		img[ "R" ] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, StringData( "hello") )
		
		w = Writer.create( img, "test/IECore/data/tiff/output.tif" )
		self.assertEqual( type(w), TIFFImageWriter )
		
		self.assertRaises( RuntimeError, w.write )
		
		
	def testWindowWrite( self ) :	
	
		dataWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)

		img = self.__makeFloatImage( dataWindow, dataWindow )
		
		img.displayWindow = Box2i(
			V2i( -20, -20 ),
			V2i( 199, 199 )
		)
		
		w = Writer.create( img, "test/IECore/data/tiff/output.tif" )
		self.assertEqual( type(w), TIFFImageWriter )		
		w.write()
		
		self.assert_( os.path.exists( "test/IECore/data/tiff/output.tif" ) )
				
		r = Reader.create( "test/IECore/data/tiff/output.tif" )
		img2 = r.read()
		
		self.assertEqual( img2.displayWindow, img.displayWindow )			
		
		ipe = PrimitiveEvaluator.create( img2 )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf ( ipe.A() )
		
		result = ipe.createResult()
				
		found = ipe.pointAtPixel( V2i( -5, -5 ), result )
		self.assert_( found )		
		color = V3f(
				result.floatPrimVar( ipe.R() ),
				result.floatPrimVar( ipe.G() ), 
				result.floatPrimVar( ipe.B() )
			)		
		expectedColor = V3f( 0, 0, 0 )
		self.assert_( ( color - expectedColor).length() < 1.e-3 )
		
		found = ipe.pointAtPixel( V2i( 99, 99 ), result )
		self.assert_( found )		
		color = V3f(
				result.floatPrimVar( ipe.R() ),
				result.floatPrimVar( ipe.G() ), 
				result.floatPrimVar( ipe.B() )
			)		
		expectedColor = V3f( 1, 1, 0 )
		self.assert_( ( color - expectedColor).length() < 1.e-3 )	
		
		
	def setUp( self ) :
		
		if os.path.exists( "test/IECore/data/tiff/output.tif" ) :
			
			os.remove( "test/IECore/data/tiff/output.tif" )					
		
		
	def tearDown( self ) :
	
		if os.path.exists( "test/IECore/data/tiff/output.tif" ) :
			
			os.remove( "test/IECore/data/tiff/output.tif" )
			
				
		
	
		
if __name__ == "__main__":
	unittest.main()   
	
