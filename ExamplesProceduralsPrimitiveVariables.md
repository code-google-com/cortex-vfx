The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Using PrimVars
#sidebar ExamplesTOC

# Using Primitive Variables #

This cookbook example demonstrates how to assign a primitive variable to a renderable. It is based very closely on the Points Render cookbook example, but adds an additional Cs primitive variable which shaders can use to colour the points.

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_primvarGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_primvarGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_primvarRI.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_primvarRI.jpg)
_renderman_

# Code #
```
#=====
# Primitive Variables
#
# This cookbook example demonstrates how to assign a primitive variable
# to a renderable. It is based very closely on the Points Render cookbook
# example, but adds an additional Cs primitive variable which shaders can
# use to colour the points.
#
#=====

from IECore import * 
from random import *

#generate a points primitive filling the bbox with npoints
def generatePoints( bbox, npoints ):
	seed(0)
	size = bbox.size()
	pdata = V3fVectorData()
	for i in range(npoints):
		pdata.append( V3f( random() * size.x + bbox.min.x,
						random() * size.y + bbox.min.y,
						random() * size.z + bbox.min.z ) )
	return PointsPrimitive( pdata )

#our primitive variable render procedural
class primitiveVariables(ParameterisedProcedural) :

	def __init__(self): 
		ParameterisedProcedural.__init__( self, "Description here." )
		bbox = Box3fParameter( "bbox", "Bounds for points.", Box3f(V3f(0), V3f(1)) )
		npoints = IntParameter( "npoints", "Number of points.", 100, minValue=0, maxValue=10000 )
		width = FloatParameter( "width", "Point width", 0.05  )
		self.parameters().addParameters( [ bbox, npoints, width ] )
		self.__points = None
		self.__npoints = None
		self.__bbox = None

	def generatePoints(self, args): 
		if args['npoints'].value!=self.__npoints or args['bbox'].value!=self.__bbox:
			self.__points = generatePoints( args['bbox'].value, args['npoints'].value )
			self.__npoints = args['npoints'].value
			self.__bbox = args['bbox'].value
		return self.__points

	def doBound(self, args): 
		self.generatePoints(args)
		return self.__points.bound()

	def doRenderState(self, renderer, args):
		pass 

	def doRender(self, renderer, args): 
		self.generatePoints(args)
		self.__points['width'] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, args['width'] )

		# create an array of colours, one per point 
		colours = []
		for i in range( self.__points['P'].data.size() ):
			colours.append( Color3f( random(), random(), random() ) )
			print colours[-1]
		colour_data = Color3fVectorData( colours )

		# attach as a Cs primitive variable 
		self.__points['Cs'] = PrimitiveVariable( PrimitiveVariable.Interpolation.Varying, colour_data )

		# render 
		self.__points.render( renderer )

#register
registerRunTimeTyped( primitiveVariables )
```