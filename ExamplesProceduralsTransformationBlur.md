The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Transformation Blur Example
#sidebar ExamplesTOC

# Transformation Blur #

This cookbook example injects multiple transformation matrices at different time samples. These transforms move a single geometry over time, causing transformation motion blur.

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_transformBlurGL.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_transformBlurGL.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_transformBlur.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_transformBlur.jpg)
_renderman_

Motion blur is defined in a very similar fashion to RenderMan. First we call `motionBegin()` with a list of time samples. You can have as many motion samples as your renderer will allow. For each time sample we then inject a transformation matrix. We finish by calling `motionEnd()`. Remember not to inject anything but transformation matrices (one per sample) between `motionBegin()` and `motionEnd()`. Be sure not to mix up deformation blur and transformation blur samples between the same `motionBegin()` and `motionEnd()` statements.

In general the code will look like:
```
renderer.motionBegin( [ sample1, sample2, ... ] )
renderer.concatTransform( matrix1 )
renderer.concatTransform( matrix2 )
...
renderer.motionEnd()
geometry.render( renderer )
```
**Note:** Don't forget to turn on motion blur in your renderer

**Note:** In this example OpenGL will render all the samples simultaneously. Refer to the [RenderMode](ExamplesProceduralsRenderingModes.md) example for code that can differentiate based on which renderer is currently rendering.

# Code #
```
 #=====
 # Transformation Blur
 #
 # This cookbook example injects multiple transformation matrices at different
 # time samples. These transforms move a single geometry over time, causing
 # transformation motion blur.
 #
 # Motion blur is defined in a very similar fashion to RenderMan. First we call
 # motionBegin() with a list of time samples. You can have as many motion samples
 # as your renderer will allow. For each time sample we then inject a
 # transformation matrix. We finish by calling motionEnd(). Remember not to
 # inject anything but transformation matrices (one per sample) between
 # motionBegin() and motionEnd(). Be sure not to mix up deformation blur and
 # transformation blur samples between the same motionBegin() and motionEnd()
 # statements.
 #
 # Don't forget to turn on motion blur in your renderer
 #
 # In OpenGL all samples will be rendered simultaneously. Refer to the
 # RenderSwitch example for code that can differentiate based on which renderer
 # is currently rendering.
 #
 # In general the code will look like:
 #
 #      renderer.motionBegin( [ sample1, sample2, ... ] )
 #      renderer.concatTransform( matrix1 )
 #      renderer.concatTransform( matrix2 )
 #      ...
 #      renderer.motionEnd()
 #      geometry.render( renderer )
 #
 #=====
 
 from IECore import *
 import IECoreGL
 import math
 
 #=====
 # return a list of floats representing a stepped range
 def frange(start, stop, step):
      width = stop - start
      n = int(round(width / step))
      return [start + step*i for i in range(n)]
 
 #=====
 # our transformation blur procedural
 class transformationBlur(ParameterisedProcedural):
 
 	#=====
 	# Init
 	def __init__(self):
 		ParameterisedProcedural.__init__( self,
 						"TransformationBlur procedural." )
 
 		# create some transformation matrices
 		self.transforms = []
 		for i in range(8):
 			self.transforms.append( M44f().createTranslated(
 									V3f( i, math.sin(i), 0 ) ) )
 
 	#=====
 	# our bounding box - this should encompass the entire blurred geometry
 	def doBound(self, args):
 		sphere_bbox = Box3f( V3f(-1,-1,-1), V3f(1,1,1) )
 		bbox = Box3f()
 		for mtx in self.transforms:
 			bbox.extendBy( sphere_bbox.min * mtx )
 			bbox.extendBy( sphere_bbox.max * mtx )
 		return bbox
 
 	#=====
 	# Nothing to do
 	def doRenderState(self, renderer, args):
 		pass
 
 	#=====
 	# Render our transform-blurred geometry
 	def doRender(self, renderer, args):
 
 		# work out samples &shutter information
 		shutter = renderer.getOption('shutter').value
 		shutter_length = shutter[1] - shutter[0]
 
 		# if we don't have motion blur enabled, make sure we still have some
 		# samples
 		if shutter_length==0:
 			shutter_length=1
 
 		# create a list of sample times
 		sample_times = frange( shutter[0], shutter[1],
 							float(shutter_length)/(len(self.transforms)) )
 
 		# draw all the samples in OpenGL
 		if renderer.typeId() == IECoreGL.Renderer.staticTypeId():
 			for mtx in self.transforms:
 				renderer.setTransform( mtx )
 				renderer.sphere( 1, -1, 1, 360, {} )
 		else:
 			# inject our transformation motion blur samples
 			renderer.motionBegin( sample_times )
 			for mtx in self.transforms:
 				renderer.concatTransform( mtx )
 			renderer.motionEnd()
 			renderer.sphere( 1, -1, 1, 360, {} )
 
 #=====
 # Register our procedural
 registerRunTimeTyped( transformationBlur )

```