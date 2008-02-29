##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
from IECore import *

class TestJPEGReader(unittest.TestCase):

	def testConstruction( self ):
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
	def testCanRead( self ):
	
		self.assert_( JPEGImageReader.canRead( "test/IECore/data/jpg/uvMap.512x256.jpg" ) )
		
	def testIsComplete( self ):	
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
		self.assert_( r.isComplete() )
		
	def testChannelNames( self ):	
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
		channelNames = r.channelNames()
		
		self.assertEqual( len( channelNames ), 3 )
		
		self.assert_( "R" in channelNames )
		self.assert_( "G" in channelNames )
		self.assert_( "B" in channelNames )				
		
	def testRead( self ):

		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )

		img = r.read()

		self.assertEqual( type(img), ImagePrimitive )
		
		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0), V2i( 511, 255) ) )
		self.assertEqual( img.dataWindow, Box2i( V2i( 0, 0), V2i( 511, 255) ) )
		
	def testReadChannel( self ):
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
		red =   r.readChannel( "R" )
		green = r.readChannel( "G" )
		blue =  r.readChannel( "B" )
		
		self.assertRaises( RuntimeError, r.readChannel, "NonExistantChannel" )
		
		self.assertEqual( len(red), len(green) )	
		self.assertEqual( len(red), len(blue) )		
		self.assertEqual( len(red), 512 * 256 )			

	def testDataWindowRead( self ):

		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
		dataWindow = Box2i(
			V2i(60, 60), 
			V2i(99, 99)
		)
		
		r.parameters().dataWindow.setValue( Box2iData( dataWindow ) )

		img = r.read()

		self.assertEqual( type(img), ImagePrimitive )
		
		self.assertEqual( img.dataWindow, dataWindow )
		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0), V2i( 511, 255) ) )
		
		self.assertEqual( len(img["R"].data), 40 * 40 )
			
	def testOrientation( self ) :
		""" Test orientation of JPEG files """
	
		img = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" ).read()
		
		ipe = PrimitiveEvaluator.create( img )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf( ipe.A() )
		
		result = ipe.createResult()
		
		# Floating point differences due to compression (?)
		# \todo Double check this.
		colorMap = {
			V2i( 0  , 0 ) :    V3f( 0, 0.00392151, 0 ),
			V2i( 511, 0 ) :    V3f( 0.984375, 0.00784302, 0 ),
			V2i( 0  , 255 ) :  V3f( 0.00784302, 0.992188, 0 ),
			V2i( 511, 255 ) :  V3f( 1, 1, 0 ),
		}
		
		for point, expectedColor in colorMap.items() :
		
			found = ipe.pointAtPixel( point, result )
			self.assert_( found )
			
			color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)
						
			self.assert_( ( color - expectedColor).length() < 1.e-6 )
			
	def testErrors( self ):
	
		r = JPEGImageReader()
		self.assertRaises( RuntimeError, r.read )
		
		# \todo We need to install a JPEG error handler!
		return	
		r = JPEGImageReader( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		img = r.read()
		
if __name__ == "__main__":
	unittest.main()   
	
