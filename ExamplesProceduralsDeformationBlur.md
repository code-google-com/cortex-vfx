The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Rendering deformation blur.
#sidebar ExamplesTOC

# Deformation Blur #
This example injects two different geometries at two different time samples creating deformation blur.

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_deformBlur_gl.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_deformBlur_gl.jpg)
_opengl_

![http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_deformBlur.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/320px-Cookbook_deformBlur.jpg)
_renderman_

Motion blur is defined in a very similar fashion to RenderMan. First we call `motionBegin()` with a list of time samples. You can have as many motion samples as your renderer will allow. For each time sample we then inject some geometry. It is important that the topology remain consistent between time samples, otherwise the renderer will complain. We finish by calling `motionEnd()`. Remember not to inject anything but geometry samples between `motionBegin()` and `motionEnd()`.

In general the code will look like:
```
 renderer.motionBegin( [ sample1, sample2, ... ] )
 sample1_geometry.render( renderer )
 sample2_geometry.render( renderer )
 ...
 renderer.motionEnd()
```

**Note:** Don't forget to turn on motion blur in your renderer!

**Note:** In OpenGL all samples will be rendered simultaneously. Refer to the [Render Mode](ExamplesProceduralsRenderingModes.md) example for code that can differentiate based on which renderer is currently rendering.

# Code #
```
 #=====
 # Deformation Blur
 #
 # This cookbook example injects two different geometries at different time
 # two time samples creating deformation blur.
 #
 # Motion blur is defined in a very similar fashion to RenderMan. First we call
 # motionBegin() with a list of time samples. You can have as many motion samples
 # as your renderer will allow. For each time sample we then inject some
 # geometry. It is important that the topology remain consistent between time
 # samples, otherwise the renderer will complain. We finish by calling
 # motionEnd(). Remember not to inject anything but geometry samples between
 # motionBegin() and motionEnd().
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
 #      sample1_geometry.render( renderer )
 #      sample2_geometry.render( renderer )
 #      ...
 #      renderer.motionEnd()
 #
 #=====
 from IECore import * 
 
 class deformationBlur(ParameterisedProcedural)&nbsp;:
 
 	#=====
 	# Init
 	def __init__(self) :
 		ParameterisedProcedural.__init__( self, "DeformationBlur procedural." )
 		geo1 = PathParameter( name="geo1", description="Geometry #1",
 							defaultValue="test_data/deform1.cob" )
 		geo2 = PathParameter( name="geo2", description="Geometry #2",
 							defaultValue="test_data/deform2.cob" )
 		self.parameters().addParameters( [geo1, geo2] )
 
 	#=====
 	# It's important that the bounding box extend to contain both geometry
 	# samples.
 	def doBound(self, args) :
 		bbox = Box3f()
 		geo1 = Reader.create( args['geo1'].value ).read()
 		geo2 = Reader.create( args['geo2'].value ).read()
 		bbox.extendBy( geo1.bound() )
 		bbox.extendBy( geo2.bound() )
 		return bbox
 
 	#=====
 	# Nothing to do
 	def doRenderState(self, renderer, args) :
 		pass
 
 	#=====
 	# Render our two motion samples
 	def doRender(self, renderer, args):
 
 		# load our geometry
 		geo1 = Reader.create( args['geo1'].value ).read()
 		geo2 = Reader.create( args['geo2'].value ).read()
 
 		# get the shutter open/close values from the renderer
 		shutter = renderer.getOption('shutter').value # this is a V2f
 
 		# if motion blur is not enabled then both shutter open & close will
 		# be zero.
 		do_moblur = ( shutter.length() > 0 )
 
 		# inject the motion samples
 		renderer.motionBegin( [ shutter[0], shutter[1] ] )
 		geo1.render( renderer )
 		geo2.render( renderer )
 		renderer.motionEnd()
 
 #=====
 # Register our procedural
 registerRunTimeTyped( deformationBlur )

```