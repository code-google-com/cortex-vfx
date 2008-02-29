##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
import os

class TestEXRReader(unittest.TestCase):

	testfile = "test/IECore/data/exrFiles/redgreen_gradient_piz_256x256.exr"
	testoutfile = "test/IECore/data/exrFiles/redgreen_gradient_piz_256x256.testoutput.exr"
	testwindowoutfile = "test/IECore/data/exrFiles/redgreen.window.exr"

	def testConstruction(self):

		r = Reader.create(self.testfile)
		self.assertEqual(type(r), EXRImageReader)


	def testRead(self) :

		r = Reader.create(self.testfile)
		self.assertEqual(type(r), EXRImageReader)

		img = r.read()

		self.assertEqual(type(img), type(ImagePrimitive() ))

		# write test
		w = Writer.create(img, self.testoutfile)
		self.assertEqual(type(w), EXRImageWriter)

		w.write()
		## \todo here we might complete the test by comparing against verified output

	def testHalf(self):

		testfile = "test/IECore/data/exrFiles/redgreen_gradient_piz_256x256.exr"

		r = Reader.create(testfile)
		self.assertEqual(type(r), EXRImageReader)

		img = r.read()
		self.assertEqual(type(img), ImagePrimitive)

	def testWindowedRead(self):

		# create a reader, read a sub-image
		r = Reader.create(self.testfile)
		self.assertEqual(type(r), EXRImageReader)
		box = Box2i(V2i(-100, -100), V2i(199, 199))
		r.parameters().dataWindow.setValue(Box2iData(box))

		# read, verify
		img = r.read()
		self.assertEqual(type(img), ImagePrimitive)

		img.displayWindow = box

		# write back the sub-image
		Writer.create(img, self.testwindowoutfile).write()
		
	def testWindowReading( self ):
	
		img = Reader.create( "test/IECore/data/exrFiles/uvMapWithDataWindow.100x100.exr" ).read()
		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 99, 99 ) ) )		
		self.assertEqual( img.dataWindow, Box2i( V2i( 25, 25 ), V2i( 49, 49 ) ) )	
		
		
	def testOrientation( self ) :
	
		img = Reader.create( "test/IECore/data/exrFiles/uvMap.512x256.exr" ).read()
		ipe = PrimitiveEvaluator.create( img )
		r = ipe.createResult()
		
		pointColors = {
			V2i(0, 0) : V3f( 0, 0, 0 ),
			V2i(511, 0) : V3f( 1, 0, 0 ),
			V2i(0, 255) : V3f( 0, 1, 0 ),
			V2i(511, 255) : V3f( 1, 1, 0 ),
		}
		
		for point, expectedColor in pointColors.items() :
		
			ipe.pointAtPixel( point, r )
			
			color = V3f( r.floatPrimVar( ipe.R() ), r.floatPrimVar( ipe.G() ), r.floatPrimVar( ipe.B() ) )
			
			self.assert_( ( color - expectedColor).length() < 1.e-6 )
		

	def tearDown(self):
			
		for f in [ self.testoutfile, self.testwindowoutfile ] :
			if os.path.isfile( f ) :	
				os.remove( f )				
				
if __name__ == "__main__":
	unittest.main()   

