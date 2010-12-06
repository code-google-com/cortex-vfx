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

from __future__ import with_statement

import unittest
import os

import nuke

import IECore
import IECoreNuke

class ParameterisedHolderTest( IECoreNuke.TestCase ) :

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
			
			if isinstance( knob, nuke.Enumeration_Knob ) :
				self.assertEqual( knob.value(), parameter.getCurrentPresetName() )
			else :
				knobValue = None
				try :
					knobValue = IECoreNuke.getKnobValue( knob )
				except :
					# not all knob types have accessors yet. some of the numeric
					# knobs don't have them because nuke has bugs and returns those
					# knobs as the wrong type. try to get the value another way.
					try :
						knobValue = knob.getValue()
					except :
						pass
				
				if knobValue is not None :
					self.assertEqual( parameter.getValue().value, knobValue )
					
	def testCreate( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		self.assertEqual( fnPH.node().name(), "procedural" )
				
		p = fnPH.getParameterised()
		
		self.failUnless( isinstance( p[0], IECore.ReadProcedural ) )
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )
				
		self.__checkParameterKnobs( p[0].parameters(), fnPH.node() )

	def testCreateWithoutVersion( self ) :
	
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural2", "read" )
		
		self.assertEqual( fnPH.node().name(), "procedural2" )
		p = fnPH.getParameterised()
		
		self.failUnless( isinstance( p[0], IECore.ReadProcedural ) )
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )
				
		self.__checkParameterKnobs( p[0].parameters(), fnPH.node() )

	def testReloadShouldntLoseValues( self ) :
		
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 1 )
		
		fnPH.node().knob( "parm_files_frame" ).setValue( 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )
		
		fnPH.node().knob( "classReload" ).execute() # trigger reload
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )

	def testReloadShouldntLoseAnimatedValues( self ) :
		
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		fnPH.node().knob( "parm_files_frame" ).setExpression( "frame" )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).hasExpression() )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).isAnimated() )
				
		fnPH.node().knob( "classReload" ).execute() # trigger reload
		
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).hasExpression() )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).isAnimated() )
				
	def testReloadShouldntLoseDefaultValues( self ) :
		
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).defaultValue(), 1 )
		
		fnPH.node().knob( "parm_files_frame" ).setValue( 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )
		
		fnPH.node().knob( "classReload" ).execute() # trigger reload
		
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).defaultValue(), 1 )
		
	def testCopyPaste( self ) :
	
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )
		
		nuke.nodeCopy( "test/IECoreNuke/parameterisedHolder.nk" )
		
		nuke.scriptClear()
	
		n = nuke.nodePaste( "test/IECoreNuke/parameterisedHolder.nk" )
		
		fnPH = IECoreNuke.FnProceduralHolder( n )
		
		p = fnPH.getParameterised()
		
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )
	
	def testRemoveClass( self ) :
	
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		p = fnPH.getParameterised()
		
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )
		
		fnPH.setProcedural( "", 0 )
		
		p = fnPH.getParameterised()
		
		self.assertEqual( p[1], "" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 0 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )
		
		for k in fnPH.node().knobs() :
		
			self.failIf( k.startswith( "parm_" ) )
	
	def testGetParameterisedHasCorrectValues( self ) :
	
		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		fnPH.node().knob( "parm_files_frame" ).setValue( 100 )
		fnPH.node().knob( "parm_files_name" ).setValue( "test" )
		fnPH.node().knob( "parm_motion_blur" ).setValue( False )
		fnPH.node().knob( "parm_bounds_specified" ).setValue( [ 0, 1, 2, 3, 4, 5 ] )
				
		self.__checkParameterKnobs( fnPH.getParameterised()[0].parameters(), fnPH.node() )
	
	def testModifyParametersAndTransferToKnobs( self ) :
	
		fnOH = IECoreNuke.FnOpHolder.create( "mult", "maths/multiply", 2 )

		self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 1 )
		self.assertEqual( fnOH.node().knob( "parm_b" ).getValue(), 2 )

		with fnOH.parameterModificationContext() as parameters :
		
			parameters["a"].setNumericValue( 10 )
			parameters["b"].setNumericValue( 20 )
			
		self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 10 )
		self.assertEqual( fnOH.node().knob( "parm_b" ).getValue(), 20 )	
				
	def tearDown( self ) :
	
		for f in [
				"test/IECoreNuke/parameterisedHolder.nk",
			] :
			
			if os.path.exists( f ) :
				os.remove( f )
				
if __name__ == "__main__":
	unittest.main()

