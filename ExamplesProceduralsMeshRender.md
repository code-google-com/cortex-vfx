The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Simple Mesh Render
#sidebar ExamplesTOC

# Simple Mesh Render #

This cookbook example demonstrates how to load & render a mesh primitive from disk using a path specified through a path parameter.

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_meshRenderGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_meshRenderGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_meshRenderRI.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_meshRenderRI.jpg)
_renderman_

# Code #

```
#=====
# Mesh Render
#
# This cookbook example demonstrates how to load & render a mesh primitive from
# disk using a path specified through a path parameter.
#
#=====

from IECore import * 

class meshRender(ParameterisedProcedural) : 

	def __init__(self) : 
 		ParameterisedProcedural.__init__( self, "Renders a mesh." )
 		path = PathParameter( "path", "Path", "" )
 		self.parameters().addParameter( path )

	def doBound(self, args) : 
 		geo = Reader.create( args['path'].value ).read()
 		return geo.bound()

	def doRenderState(self, renderer, args) : 
 		pass

	def doRender(self, renderer, args) : 
 		geo = Reader.create( args['path'].value ).read()
 		geo.render( renderer )

#register
registerRunTimeTyped( meshRender )
```