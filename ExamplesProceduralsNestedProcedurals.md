The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Rendering with nested procedurals
#sidebar ExamplesTOC

# Nested Procedurals #
This cookbook example demonstrates how to inject many child procedurals from a single parent procedural. This can make a lot of sense if your child procedural is very heavy and you only want to use it when it really is required by the renderer.

Here we create a list of random point positions and inject the nestedChild procedural for each point in the list.

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_nestedProceduralGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_nestedProceduralGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_nestedProceduralRI.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_nestedProceduralRI.jpg)
_renderman_

# Code #
```
#=====
# Nested Child
#
# This cookbook example renders a unit cube. It is designed to be called by
# the nestedParent example to demonstrate how to inject nested procedurals.
#
#=====

from IECore import *

class nestedChild(ParameterisedProcedural) :

	def __init__(self) :
		ParameterisedProcedural.__init__( self, "Child procedural." )

	def doBound(self, args) :
		return Box3f( V3f( -.5 ), V3f( .5 ) )

	def doRenderState(self, renderer, args) :
		pass

	def doRender(self, renderer, args) :
		MeshPrimitive.createBox( Box3f( V3f( -.5 ), V3f( .5 ) ) ).render( renderer )

# register
registerRunTimeTyped( nestedChild )
```

---

```
#=====
# Nested Parent
#
# This cookbook example demonstrates how to inject many child procedurals from
# a single parent procedural.
#
# Here we create a random point cloud and inject the nestedChild procedural
# for each point.
#
#=====

from IECore import *
from random import *
import IECoreGL

class nestedParent(ParameterisedProcedural) :

	def __init__(self) :
		ParameterisedProcedural.__init__( self, "Description here." )
		self.__pdata = []
		seed(0)
		for i in range(100):
			self.__pdata.append( V3f( random()*10, random()*10, random()*10 ) )

	def doBound(self, args) :
		return Box3f( V3f( 0 ), V3f( 10 ) )

	def doRenderState(self, renderer, args) :
		pass

	def doRender(self, renderer, args) :
		# loop through our points
		for p in self.__pdata:

			# push the transform state
			renderer.transformBegin()

			# concatenate a transformation matrix
			renderer.concatTransform( M44f().createTranslated( p ) )

			# create an instance of our child procedural
			procedural = ClassLoader.defaultProceduralLoader().load( "nestedChild", 1 )()

			# do we want to draw our child procedural immediately or defer
			# until later?
			immediate_draw = False
			if renderer.typeId()==IECoreGL.Renderer.staticTypeId():
				immediate_draw = True

			# render our child procedural
			procedural.render( renderer, withGeometry=True, immediateGeometry=immediate_draw )

			# pop the transform state
			renderer.transformEnd()

# register
registerRunTimeTyped( nestedParent )
```