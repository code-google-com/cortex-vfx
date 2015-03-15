The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary subdiv render
#sidebar ExamplesTOC

# Subdivision Surface Render #

This cookbook example demonstrates how to load & render a mesh primitive and render it as a subdivision surface by setting the mesh primitive interpolation to "catmullClark".

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_subdRenderGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_subdRenderGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_subdRenderRI.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_subdRenderRI.jpg)
_renderman_

# Code #
```
#=====
# Subd Render
#
# This cookbook example demonstrates how to load & render a mesh primitive and
# render it as a subdivision surface.
#
#=====

from IECore import *

class subdRender(ParameterisedProcedural) :

	def __init__(self) :
		ParameterisedProcedural.__init__( self, "Renders a mesh as a subd." )
		path = PathParameter( "path", "Path", "" )
		self.parameters().addParameter( path )

	def doBound(self, args) :
		geo = Reader.create( args['path'].value ).read()
		return geo.bound()

	def doRenderState(self, renderer, args) :
		pass

	def doRender(self, renderer, args) :
		geo = Reader.create( args['path'].value ).read()
		geo.interpolation = "catmullClark"
		geo.render( renderer )

# register
registerRunTimeTyped( subdRender )
```