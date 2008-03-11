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
import glob
from IECore import *

from math import pow

class TestCINReader(unittest.TestCase):

	testfile =    "test/IECore/data/cinFiles/bluegreen_noise.cin"
	testoutfile = "test/IECore/data/cinFiles/bluegreen_noise.testoutput.cin"

	def testConstruction(self):

		r = Reader.create(self.testfile)
		self.assertEqual(type(r), CINImageReader)

	def testRead(self):

		r = Reader.create(self.testfile)
		self.assertEqual(type(r), CINImageReader)

		img = r.read()
		
		self.assertEqual(type(img), ImagePrimitive)

	def testWindowedRead(self):

		testfile = ["test/IECore/data/cinFiles/bluegreen_noise", "cin"]

		test_file_path = ".".join(testfile)
		test_outfile_path = ".".join([testfile[0], 'windowtestoutput', testfile[1]])

		# create a reader, read a sub-image
		r = Reader.create(test_file_path)
		self.assertEqual(type(r), CINImageReader)
		r.parameters().dataWindow.setValue(Box2iData(Box2i(V2i(100, 100), V2i(199, 199))))

		# read, verify
		img = r.read()
		self.assertEqual(type(img), ImagePrimitive)

		# write back the sub-image
		w = Writer.create(img, test_outfile_path)
		self.assertEqual(type(w), CINImageWriter)
		w.write()

	def testChannelRead(self):

		testfile = ["test/IECore/data/cinFiles/bluegreen_noise", "cin"]

		test_file_path = ".".join(testfile)
		test_outfile_path = ".".join([testfile[0], 'channeltestoutput', testfile[1]])

		# create a reader, constrain to a sub-image, R (red) channel
		r = Reader.create(test_file_path)
		self.assertEqual(type(r), CINImageReader)
		
		r.parameters().dataWindow.setValue(Box2iData(Box2i(V2i(100, 100), V2i(199, 199))))
		r.parameters().channels.setValue(StringVectorData(["R", "G"]))

		# read, verify
		img = r.read()
		self.assertEqual(type(img), ImagePrimitive)

		# write back the sub-image
		w = Writer.create(img, test_outfile_path)
		self.assertEqual(type(w), CINImageWriter)
		w.write()
		
	def testOrientation( self ) :
		""" Test orientation of Cineon files """
	
		img = Reader.create( "test/IECore/data/cinFiles/uvMap.512x256.cin" ).read()
		
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
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)
			
			print ( color, expectedColor )
						
			self.assert_( ( color - expectedColor).length() < 1.e-6 )	
		
	def testAll( self ):
		
		fileNames = glob.glob( "test/IECore/data/cinFiles/*.cin" )
		expectedFailures = []
		
		# Silence any warnings while the tests run
		MessageHandler.pushHandler( NullMessageHandler() )
		
		
		try:
		
			for f in fileNames:
			
				if not f in expectedFailures :
				
					print( f )

					r = CINImageReader( f ) 
					img = r.read()
					self.assertEqual( type(img), ImagePrimitive )
					self.assert_( img.arePrimitiveVariablesValid() )	
				
		except:
		
			raise	
			
		finally:
			
			MessageHandler.popHandler()		
		

                			
if __name__ == "__main__":
	unittest.main()   
	
