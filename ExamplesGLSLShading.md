The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Applying GLSL shaders to a procedural
#sidebar ExamplesTOC

# Introduction #

By default the OpenGL preview display for a procedural uses a simple facing ratio shader. This can be overridden with any GLSL shader to preview shading, lighting or to visualise primitive variables. IECoreGL provides a few header files which make it simple to calculate per-pixel lighting from the legacy GL lights defined in the Maya viewport. It also provides facilities for automatically mapping primitive variables to vertex shader parameters and texture files and splines into samplers.

# Example procedural #

The heavily commented code below provides a simple example of applying diffuse shading to a sphere, such that it interacts with the lights in the Maya viewport.

```
import IECore

# IECoreGL will either load shaders from disk, from text files called shaderName.vert and 
# shaderName.frag, found in directories specified using the IECOREGL_SHADER_PATHS environment
# variable, OR the source can be specified directly as string variables to the renderer.
# We'll opt for the latter case as keeping the shader code internal to the example simplifies
# things a bit. What follows is just plain old GLSL, with the addition of a facility for
# #including header files provided by Cortex.

vertexSource = """
varying vec3 P;
varying vec3 N;
		
void main()
{
	vec4 pCam = gl_ModelViewMatrix * gl_Vertex;
	P = pCam.xyz;
	gl_Position = gl_ProjectionMatrix * pCam;
	N = normalize( gl_NormalMatrix * gl_Normal );
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
"""

fragmentSource = """
#include "IECoreGL/Diffuse.h"
#include "IECoreGL/Lights.h"

varying vec3 N;
varying vec3 P;

varying vec2 textureCoordinates;

uniform float Kd;
uniform vec3 colour;

uniform bool useTexture;
uniform sampler2D texture;

void main()
{
	vec3 Cl[gl_MaxLights];
	vec3 L[gl_MaxLights];
	
	lights( P, Cl, L, 4 );
	vec3 d = Kd * colour * ieDiffuse( P, N, Cl, L, 4 );
	
	if( useTexture )
	{
		d *= texture2D( texture, gl_TexCoord[0].xy ).rgb;
	}
	
	gl_FragColor = vec4( d, 1.0 );
}
"""

# This will be the noddiest procedural ever, just rendering a sphere. But
# it'll also demonstrate how to assign shaders using GLSL.
class glslExample( IECore.ParameterisedProcedural ) :

	def __init__( self ) :

		IECore.ParameterisedProcedural.__init__( self )

		self.parameters().addParameters(

			[
				
				IECore.FloatParameter(
					name = "Kd",
					description = "How much diffuse to have.",
					defaultValue = 1,
				),
								
				IECore.Color3fParameter(
					name = "colour",
					description = "The colour of the objects.",
					defaultValue = IECore.Color3f( 1 ),
				),
				
				IECore.FileNameParameter(
					name = "texture",
					description = "A texture to apply.",
					defaultValue = "",
				),

			],
			
		)
		
	def doBound( self, args ) :

		return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )

	def doRenderState( self, renderer, args ) :
	
		pass

	def doRender( self, renderer, args ) :

		print args["colour"].typeName()

		if renderer.typeName() == "IECoreGL::Renderer" :
		
			# Shaders are specified to the OpenGL renderer in exactly
			# the same manner as they are to RenderMan or Arnold, using
			# the shader method.
			renderer.shader(
				"surface",
				# In this case the name of our shader is irrelevant, as
				# we're providing the source code in the "gl:vertexSource"
				# and "gl:fragmentSource" parameters below. If we didn't
				# provide those parameters, then the name would specify
				# the name of the shaders to load from the IECOREGL_SHADER_PATHS
				# on disk.
				"test",
				# Parameters passed to the shader call are automatically mapped
				# to uniform parameters of the GLSL shader. Textures are automatically
				# loaded from disk as necessary and provided to samplers in the shader.
				# Although not demonstrated here, you can also pass SplineData as a
				# shader parameter, and it'll automatically be rasterised and converted
				# to a texture too.
				{
					"Kd" : args["Kd"],
					"colour" : args["colour"],
					"useTexture" : IECore.BoolData( True if args["texture"].value else False ),
					"texture" : args["texture"],
					"gl:vertexSource" : IECore.StringData( vertexSource ),
					"gl:fragmentSource" : IECore.StringData( fragmentSource ),			
				}
			)

		# A sphere!
		renderer.sphere( 1, -1, 1, 360, {} )
		
IECore.registerRunTimeTyped( glslExample )
```

# Hosting in Maya #

In Maya, the lighting context in which the GLSL is run is built from the Maya lights, so the GLSL shader can do per-pixel lighting.

![http://cortex-vfx.googlecode.com/svn/wiki/images/Cookbook_glsl.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/Cookbook_glsl.jpg)