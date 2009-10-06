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

from IECore import *
import os

class CachedReaderTest( unittest.TestCase ) :

	def test( self ) :

		r = CachedReader( SearchPath( "./", ":" ), 100 * 1024 * 1024 )

		o = r.read( "test/IECore/data/cobFiles/compoundData.cob" )
		self.assertEqual( o.typeName(), "CompoundData" )
		self.assertEqual( r.memoryUsage(), o.memoryUsage() )

		oo = r.read( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( r.memoryUsage(), o.memoryUsage() + oo.memoryUsage() )

		oo = r.read( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( r.memoryUsage(), o.memoryUsage() + oo.memoryUsage() )

		self.assertRaises( RuntimeError, r.read, "doesNotExist" )
		self.assertRaises( RuntimeError, r.read, "doesNotExist" )

		# Here, the cache should throw away "o" (because there isn't enough room for it, and it was the least
		# recently used) leaving us with just "oo"
		r.maxMemory = oo.memoryUsage() + ( o.memoryUsage() / 2 )
		self.assertEqual( r.memoryUsage(), oo.memoryUsage() )

		# Here, the cache should throw away "oo" (because there isn't enough room for it, and it was the least
		# recently used) leaving us empty
		r.maxMemory = oo.memoryUsage() / 2
		self.assertEqual( r.memoryUsage(), 0 )

		r.maxMemory = oo.memoryUsage() * 2
		oo = r.read( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		r.clear()
		self.assertEqual( r.memoryUsage(), 0 )

		oo = r.read( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( r.memoryUsage(), oo.memoryUsage() )
		r.clear( "I don't exist" )
		self.assertEqual( r.memoryUsage(), oo.memoryUsage() )
		r.clear( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( r.memoryUsage(), 0 )

		# testing insert.
		r.insert( "test/IECore/data/pdcFiles/particleShape1.250.pdc", oo )
		self.assertEqual( r.memoryUsage(), oo.memoryUsage() )
		o2 = r.read( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( oo, o2 )

	def testDefault( self ) :

		os.environ["IECORE_CACHEDREADER_PATHS"] = "a:test:path"

		r = CachedReader.defaultCachedReader()
		r2 = CachedReader.defaultCachedReader()
		self.assert_( r.isSame( r2 ) )
		self.assertEqual( r.maxMemory, 1024 * 1024 * 100 )
		self.assertEqual( r.searchPath, SearchPath( "a:test:path", ":" ) )

if __name__ == "__main__":
    unittest.main()
