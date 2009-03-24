##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
import sys
import glob
from IECore import *

from math import pow

class SGIImageReaderTest(unittest.TestCase):

	def testConstruction(self):

		r = Reader.create( "test/IECore/data/sgiFiles/uvMap.512x256.8bit.sgi" )
		self.assertEqual(type(r), SGIImageReader)

	def testReadHeader( self ):

		r = Reader.create( "test/IECore/data/sgiFiles/uvMap.512x256.8bit.sgi" )
		self.assertEqual( type(r), SGIImageReader )
		h = r.readHeader()

		channelNames = h['channelNames']		
		self.assertEqual( len( channelNames ), 3 )
		self.assert_( "R" in channelNames )
		self.assert_( "G" in channelNames )
		self.assert_( "B" in channelNames )

		self.assertEqual( h['displayWindow'], Box2iData( Box2i( V2i(0,0), V2i(511,255) ) ) )
		self.assertEqual( h['dataWindow'], Box2iData( Box2i( V2i(0,0), V2i(511,255) ) ) )

	def testRead(self):

		r = Reader.create( "test/IECore/data/sgiFiles/uvMap.512x256.8bit.sgi" )
		self.assertEqual(type(r), SGIImageReader)

		img = r.read()
		
		self.assertEqual(type(img), ImagePrimitive)

	def testDataWindowRead( self ):
	
		for f in glob.glob( "test/IECore/data/sgiFiles/uvMap.512x256.*sgi" ) :

			r = Reader.create( f )
			self.assertEqual( type(r), SGIImageReader )

			dataWindow = Box2i(
				V2i(360, 160), 
				V2i(399, 199)
			)

			r.parameters()["dataWindow"].setValue( Box2iData( dataWindow ) )

			img = r.read()

			self.assertEqual( type(img), ImagePrimitive )

			self.assertEqual( img.dataWindow, dataWindow )
			self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )

			self.assertEqual( len(img["R"].data), 40 * 40 )

			ipe = PrimitiveEvaluator.create( img )
			self.assert_( ipe.R() )
			self.assert_( ipe.G() )
			self.assert_( ipe.B() )
			self.failIf ( ipe.A() )

			result = ipe.createResult()

			# Check that the color at the bottom-right of the image is black - ordinarialy it would
			# be yellow, but we're deliberately not reading the entire image
			found = ipe.pointAtPixel( V2i( 511, 255 ), result )
			self.assert_( found )		
			color = V3f(
					result.floatPrimVar( ipe.R() ),
					result.floatPrimVar( ipe.G() ), 
					result.floatPrimVar( ipe.B() )
				)		
			expectedColor = V3f( 0, 0, 0 )
			self.assert_( ( color - expectedColor).length() < 1.e-3 )

			found = ipe.pointAtPixel( V2i( 380, 180 ), result )
			self.assert_( found )

			color = V3f(
					result.floatPrimVar( ipe.R() ),
					result.floatPrimVar( ipe.G() ), 
					result.floatPrimVar( ipe.B() )
				)

			expectedColor = V3f( 0.741176, 0.705882, 0 )		
			
			if not ( ( color - expectedColor).length() < 1.e-2 ):
			
				print f,   color, expectedColor
			
			self.assert_( ( color - expectedColor).length() < 1.e-2 )

	def testChannelRead(self):

		# create a reader, constrain to a sub-image, R and G channels
		r = Reader.create( "test/IECore/data/sgiFiles/uvMap.512x256.8bit.sgi" )
		self.assertEqual( type(r), SGIImageReader )
		
		r.parameters()["dataWindow"].setValue( Box2iData( Box2i( V2i(100, 100), V2i(199, 199) ) ) )
		r.parameters()["channels"].setValue( StringVectorData( ["R", "G"] ) )

		img = r.read()
		self.assertEqual( type(img), ImagePrimitive )
		
	def testOrientation( self ) :
		""" Test orientation of SGI files """
	
		for f in glob.glob( "test/IECore/data/sgiFiles/uvMap.512x256.*sgi" ) :

			r = Reader.create( f )
			img = r.read()
		
			ipe = PrimitiveEvaluator.create( img )
			self.assert_( ipe.R() )
			self.assert_( ipe.G() )
			self.assert_( ipe.B() )
			self.failIf ( ipe.A() )

			result = ipe.createResult()

			colorMap = {
				V2i( 0 ,    0 ) :  V3f( 0, 0, 0 ),
				V2i( 511,   0 ) :  V3f( 1, 0, 0 ),
				V2i( 0,   255 ) :  V3f( 0, 1, 0 ),
				V2i( 511, 255 ) :  V3f( 1, 1, 0 ),
			}

			for point, expectedColor in colorMap.items() :

				found = ipe.pointAtPixel( point, result )
				self.assert_( found )

				color = V3f(
					result.floatPrimVar( ipe.R() ),
					result.floatPrimVar( ipe.G() ), 
					result.floatPrimVar( ipe.B() )
				)
				
				if not ( ( color - expectedColor).length() < 1.e-2 ):
			
					print f,   color, expectedColor

				self.assert_( ( color - expectedColor).length() < 1.e-2 )	
		
	def testAll( self ):
		
		fileNames = glob.glob( "test/IECore/data/sgiFiles/*.sgi" )
		expectedFailures = []
		
		# Silence any warnings while the tests run
		MessageHandler.pushHandler( NullMessageHandler() )
		
		
		try:
		
			for f in fileNames:
			
				r = SGIImageReader( f ) 
				
				if f in expectedFailures :
					
					self.assertRaises( RuntimeError, r.read )
				
				else :
				
					self.assert_( SGIImageReader.canRead( f ) )
					self.failIf( JPEGImageReader.canRead( f ) )
					self.failIf( EXRImageReader.canRead( f ) )
					self.failIf( TIFFImageReader.canRead( f ) )					
					self.failIf( CINImageReader.canRead( f ) )										

					img = r.read()
					self.assertEqual( type(img), ImagePrimitive )
					self.assert_( img.arePrimitiveVariablesValid() )	
				
		except:
		
			raise	
			
		finally:
			
			MessageHandler.popHandler()		
		

                			
if __name__ == "__main__":
	unittest.main()   
	
