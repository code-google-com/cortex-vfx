The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Using rendering modes
#sidebar ExamplesTOC

# Rendering Modes #

This example demonstrates how to render based on the current rendering context - specifically differentiating between OpenGL and RenderMan. This procedural renders a cube in OpenGL but a sphere in RenderMan.

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_renderModeGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_renderModeGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_renderModeRI.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_renderModeRI.jpg)
_renderman_

# Code #
```
 #=====
 # Render Mode
 #
 # This cookbook example demonstrates how to render based on the current
 # rendering context - specifically differentiating between OpenGL and RenderMan.
 # This procedural renders a cube in OpenGL but a sphere in RenderMan.
 #
 #=====
 
 from IECore import *
 import IECoreGL
 import IECoreRI
 
 class renderMode(ParameterisedProcedural)&nbsp;:
 
 	def __init__(self)&nbsp;:
 		ParameterisedProcedural.__init__( self, "RenderMode cookbook example." )
 
 	def doBound(self, args)&nbsp;:
 		return Box3f( V3f( -1, -1, -1 ), V3f( 1, 1, 1 ) )
 
 	def doRenderState(self, renderer, args)&nbsp;:
 		pass
 
 	def doRender(self, renderer, args)&nbsp;:
 
 		# This checks the renderer against the GL renderer type
 		if renderer.typeId()==IECoreGL.Renderer.staticTypeId():
 			MeshPrimitive.createBox( Box3f( V3f(-1), V3f(1) ) ).render( renderer )
 
 		# This checks the renderer against the RenderMan renderer type
 		if renderer.typeId()==IECoreRI.Renderer.staticTypeId():
 			renderer.sphere( -1, 1, -1, 360, {} )
 
 # register
 registerRunTimeTyped( renderMode )
```