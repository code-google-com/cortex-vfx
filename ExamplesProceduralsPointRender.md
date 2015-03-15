The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary simple point render
#sidebar ExamplesTOC

# Point Render #
This cookbook example demonstrates how to create and render a Cortex PointsPrimitive. The procedural creates a points primitive and fills it with a specified number of points, within a specified bounding box.

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_pointsRenderGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_pointsRenderGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_pointsRenderRI.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_pointsRenderRI.jpg)
_renderman_

# Code #
```
#=====
# Point Render
#
# This cookbook example demonstrates how to create and render a Cortex
# PointsPrimitive. The procedural creates a points primitive and fills it with
# a specified number of points, within a specified bounding box.
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

#our point render procedural
class pointRender(ParameterisedProcedural) :
	def __init__(self) : 
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

	def doBound(self, args) : 
		self.generatePoints(args)
		return self.__points.bound()

	def doRenderState(self, renderer, args) : 
		pass

	def doRender(self, renderer, args) : 
		self.generatePoints(args)
		self.__points['width'] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, args['width'] )
		self.__points.render( renderer )

#register
registerRunTimeTyped( pointRender )
```