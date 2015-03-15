The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Simple geo example
#sidebar ExamplesTOC

# Simple Procedural Geometry #
This example is just about the simplest procedural you can get. It renders a cube!

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_simpleGeometryGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_simpleGeometryGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_simpleGeometry.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_simpleGeometry.jpg)
_renderman_

# Code #
```
 #=====
 # Simple Geometry
 #
 # This example is just about the simplest procedural you can get. It renders
 # a cube!
 #=====
 from IECore import *
 
 class simpleGeometry(ParameterisedProcedural) :
 
 	def __init__(self) :
 		ParameterisedProcedural.__init__( self, "Simple Geometry." )
 
 	def doBound(self, args) :
 		return Box3f( V3f(-1), V3f(1) )
 
 	def doRenderState(self, renderer, args) :
 		pass
 
 	def doRender(self, renderer, args) :
 		box = MeshPrimitive.createBox( Box3f( V3f(-1), V3f(1) ) )
 		box.render( renderer )
 
 # register
 registerRunTimeTyped( simpleGeometry )
```