##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreNuke

class ParameterisedHolderTest( unittest.TestCase ) :

	def __checkParameterKnobs( self, parameter, node, knobName=None ) :
	
		if knobName is None :
			knobName = "parm"
	
		if isinstance( parameter, IECore.CompoundParameter ) :
			for k in parameter.keys() :
				childKnobName = knobName + "_" + parameter[k].name
				self.__checkParameterKnobs( parameter[k], node, childKnobName )
		else :
			
			knob = node.knob( knobName )
			self.failUnless( knob is not None )
			
			knobValue = None
			try :
				knobValue = IECoreNuke.getKnobValue( knob )
			except :
				# not all knob types have accessors yet
				pass
				
			if knobValue is not None :

				parameterValue = parameter.getValue()
				self.assertEqual( parameterValue.value, knobValue )
					
	def testCreate( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		self.assertEqual( fnPH.node().name(), "procedural" )
		p = fnPH.getParameterised()
		
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )
		
		procedural = IECore.ClassLoader.defaultProceduralLoader().load( "read", 1 )()
		
		self.__checkParameterKnobs( procedural.parameters(), fnPH.node() )

	def testCreateWithoutVersion( self ) :
	
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural2", "read" )
		
		self.assertEqual( fnPH.node().name(), "procedural2" )
		p = fnPH.getParameterised()
		
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )
		
		procedural = IECore.ClassLoader.defaultProceduralLoader().load( "read", 1 )()
		
		self.__checkParameterKnobs( procedural.parameters(), fnPH.node() )

	def testReloadShouldntLoseValues( self ) :
		
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 1 )
		
		fnPH.node().knob( "parm_files_frame" ).setValue( 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )
		
		fnPH.setProcedural( "read", 1 ) # trigger reload
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )

	def testReloadShouldntLoseAnimatedValues( self ) :
		
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		fnPH.node().knob( "parm_files_frame" ).setExpression( "frame" )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).hasExpression() )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).isAnimated() )
				
		fnPH.setProcedural( "read", 1 ) # trigger reload
		
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).hasExpression() )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).isAnimated() )
				
	def testReloadShouldntLoseDefaultValues( self ) :
		
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).defaultValue(), 1 )
		
		fnPH.node().knob( "parm_files_frame" ).setValue( 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )
		
		fnPH.setProcedural( "read", 1 ) # trigger reload
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).defaultValue(), 1 )

if __name__ == "__main__":
	unittest.main()

