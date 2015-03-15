The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary An example procedural showing a simple hair system with GPU based Marschner shading.

![http://opensource.image-engine.com/images/examples/shadedHairExample.jpg](http://opensource.image-engine.com/images/examples/shadedHairExample.jpg)

![http://opensource.image-engine.com/images/examples/basicHairExampleAE.png](http://opensource.image-engine.com/images/examples/basicHairExampleAE.png)

# Introduction #

An example of a simple hair system inside a procedural, complete with Marschner based hair shading. Sadly no shadows yet.

It demonstrates a couple of concepts:

  * Parameter checksumming to improve interactive performance.
  * Use of glsl shaders with textures.
  * Use of IECore point distribution functions to seed hairs.

You may need to download:

  * http://johanneskopf.de/publications/blue_noise/tilesets/tileset_2048.dat

and set **$CORTEX\_POINTDISTRIBUTION\_TILESET** accordingly, if you haven't already one so.

# Code #

```
import IECore

class ShadedHairExample( IECore.ParameterisedProcedural ) :

        def __init__( self ) :

                IECore.ParameterisedProcedural.__init__( self )

                self.parameters().addParameters(

                        [
                                ## Connect your own mesh to grow hair on here, if you like.
                                IECore.ObjectParameter(
                                        name = "mesh",
                                        description = "The mesh to grow hair on, it should have well laid out UVs.",
                                        defaultValue = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2.0 ), IECore.V2f( 2.0 ) ) ),
                                        types = [IECore.TypeId.MeshPrimitive]
                                ),
                                                                                                        
                                ## Add in some controls for the glsl shader parameters.
                                
                                IECore.FloatParameter(
                                        name = "diffuse",
                                        description = "Strength of the 'fake' diffuse shading.",
                                        defaultValue = 0.4,
                                        minValue = 0.0,
                                        maxValue = 1.0
                                ),
                                
                                IECore.FloatParameter(
                                        name = "whiteSpec",
                                        description = "The strength of the shading conrtribution from the 'R' component of the Marschner model.",
                                        defaultValue = 2.5,
                                        minValue = 0.0,
                                        maxValue = 5.0
                                ),
                                
                                IECore.FloatParameter(
                                        name = "backlight",
                                        description = "The strength of the shading conrtribution from the 'TT' component of the Marschner model.",
                                        defaultValue = 0.9,
                                        minValue = 0.0,
                                        maxValue = 5.0
                                ),
                                
                                IECore.FloatParameter(
                                        name = "colouredSpec",
                                        description = "The strength of the shading conrtribution from the 'TRT' component of the Marschner model.",
                                        defaultValue = 1.0,
                                        minValue = 0.0,
                                        maxValue = 5.0
                                ),
                                
                                ## We can take advantage of the MarschnerParameter to add in the controls for the base
                                ## Spec model, which is used to generate the lookup tables for GPU shading.
                                
                                IECore.MarschnerParameter( name = "marschnerParameters" ),
                                
                                ## Add some controls for hair generation.
                                
                                IECore.CompoundParameter( 
                                
                                        name = "hair",
                                        members = [
                                
                                                IECore.FloatParameter(
                                                        name = "length",
                                                        description = "The length of the hair, in world units.",
                                                        defaultValue = 4.0
                                                ),

                                                IECore.IntParameter(
                                                        name = "count",
                                                        description = "The numbe of hairs to generate.",
                                                        defaultValue = 5000
                                                ),
                                                
                                                IECore.ClassVectorParameter(
                                                    name = "modifiers",
                                                    description = "Components to modify the hair.",
                                                    searchPathEnvVar = "HAIR_MODIFIER_PATHS",
                                                    userData = {
                                                        "UI" : {
                                                            "classNameFilter" : IECore.StringData( "*" ),
                                                        },
                                                    },
                                                ),

                                       ]
                                )
                        ]
                )
                
                ## Change the default colour for the hair in the marschner model.
                self.parameters()["marschnerParameters"]["color"].setTypedValue( IECore.Color3f( 0.4, 0.2, 0.05 ) )
                
                ## We are going to cache our lookup tables, and the hair, so were
                ## not needlessly re-calculating things that haven't changed.
                self.__hairChecksum = -1
                self.__shadingChecksum = -1


        def doBound( self, args ) :
                
                ## We don't actually know where the hair will be, but
                ## we do know it can't get any longer than a certain length
                ## so just expand the bounding box by that amount to be safe.
                
                box = args["mesh"].bound()
                box.min -= IECore.V3f( args["hair"]["length"].value );
                box.max += IECore.V3f( args["hair"]["length"].value );
                return box


        def doRenderState( self, renderer, args ) :
                pass


        def doRender( self, renderer, args ) :

                args["mesh"].render( renderer )
                
                ## See if we need to re-calculate the hair. The host layer should be
                ## keeping track on the update counts of parameters, so we'll use those
                ## as a checksum to see if anything relevant has been altered. If the 
                ## checksum is different to last time, we need to re-calculate.
                
                hairChecksum = 0        
                
                if "updateCount" in self.parameters()["hair"].userData() :
                        hairChecksum += self.parameters()["hair"].userData()["updateCount"].value
                
                if "updateCount" in self.parameters()["mesh"].userData() :
                        hairChecksum += self.parameters()["mesh"].userData()["updateCount"].value
                        
                if hairChecksum != self.__hairChecksum :
                                        
                        self.hair = self.generateHair( args["mesh"], args["hair"] )
                        self.__hairChecksum = hairChecksum
                
                ## For now, set a very basic constant color on the hair, using the color set in 
                ## the shading model.
                self.hair["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, args["marschnerParameters"]["color"] )
                
                ## Using a scoped attribute block to make sure the any changes we make are contained.
                ## This isn't strictly necessary as the procedural implementation should preserve/restore
                ## state either side of the entry/exit into each individual procedural.
                with IECore.AttributeBlock( renderer ) :
                        
                        if renderer.typeName()=="IECoreGL::Renderer":

                                # Set up how our hair will render
                                self.__setupGLRenderer( renderer )

                                # Set up our shading
                                self.__setupGLShader( renderer, args )

                        # Render the hair we calculated earlier 
                        self.hair.render( renderer )

        
        ## This simply sets up a couple of attributes to control how things will render
        def __setupGLRenderer( self, renderer ):
                        
                renderer.setAttribute( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) )
                renderer.setAttribute( "gl:curvesPrimitive:ignoreBasis", IECore.BoolData( True ) )
                
                ## The draw states prevent the wireframe being drawn on top of the hair 
                ## when selected, which obscures it.
                renderer.setAttribute( "gl:primitive:wireframe", IECore.BoolData( False ) )
                renderer.setAttribute( "gl:primitive:solid", IECore.BoolData( True ) )


        def __setupGLShader( self, renderer, args ):
                
                ## Do the same checksumming we did with the hair for the Marschner parameters
                ## to allow us to re-use the lookup tables where possible.
                
                shadingChecksum = 0
        
                shadingParam = self.parameters()["marschnerParameters"]
                if "updateCount" in shadingParam.userData() :
                        shadingChecksum = shadingParam.userData()["updateCount"].value
                
                if shadingChecksum != self.__shadingChecksum :
                
                        ## This will returns us an image, with all of the required channels for
                        ## the lookup. We need to split these up into separate RGBA textures 
                        ## so we can send them to the graphics card.
                        lookup = IECore.MarschnerLookupTableOp()( model=args["marschnerParameters"] )

                        ## Currently, there is no easy way to take an ImagePrimitive and set it as
                        ## a shader parameter. There is however an implicit conversion in the GL renderer 
                        ## between a CompoundData object with the right hierarchy, and a gl texture. 
                        ## So, we need to take the channel data out of the image primitive, and build a 
                        ## a compound data representation, and feed that into the appropriate shader parameter.

                        dw = IECore.Box2iData( lookup.dataWindow )

                        tex1 = IECore.CompoundData()
                        tex2 = IECore.CompoundData()
                        tex3 = IECore.CompoundData()

                        tex1["displayWindow"] = dw
                        tex1["dataWindow"] = dw
                        tex2["displayWindow"] = dw
                        tex2["dataWindow"] = dw
                        tex3["displayWindow"] = dw
                        tex3["dataWindow"] = dw

                        tex1["channels"] = IECore.CompoundData()
                        tex2["channels"] = IECore.CompoundData()
                        tex3["channels"] = IECore.CompoundData()

                        tex1["channels"]["R"] = lookup["MR"].data
                        tex1["channels"]["G"] = lookup["MTT"].data
                        tex1["channels"]["B"] = lookup["MTRT"].data
                        tex1["channels"]["A"] = lookup["cosDiffTheta"].data

                        tex2["channels"]["R"] = lookup["NTT.r"].data
                        tex2["channels"]["G"] = lookup["NTT.g"].data
                        tex2["channels"]["B"] = lookup["NTT.b"].data
                        tex2["channels"]["A"] = lookup["NR"].data

                        tex3["channels"]["R"] = lookup["NTRT.r"].data
                        tex3["channels"]["G"] = lookup["NTRT.g"].data
                        tex3["channels"]["B"] = lookup["NTRT.b"].data

                        ## Store these so we can re-use them later if we want to save some time
                        self.__lookup1 = tex1
                        self.__lookup2 = tex2
                        self.__lookup3 = tex3
                        
                        self.__shadingChecksum = shadingChecksum

                ## We can now feed these image representations to the shader via its parameters
                ## These names need to match those declared in the vert/frag glsl shader.
                shaderArgs = {
                        "lookupM" : self.__lookup1,
                        "lookupN" : self.__lookup2,
                        "lookupNTRT" : self.__lookup3,
                        "scaleR" : args["whiteSpec"],
                        "scaleTT" : args["backlight"],
                        "scaleTRT" : args["colouredSpec"],
                        "scaleDiffuse" : args["diffuse"],
                        "diffuseFalloff" : IECore.FloatData( 0.4 ),
                        "diffuseAzimuthFalloff" : IECore.FloatData( 0.4 )
                }
                
                ## You may need to adjust this shader path, depending on your cortex installation.
                renderer.shader( "surface", "IECoreGL/5/ieMarschnerHair", shaderArgs )


        ## A super-simple hair system, that simply grows along the normals, 
        ## with a little gravity and frizz. 
        
        def generateHair( self, mesh, args ) :
                                
                ## This returns us a PointsPrimtive, which we are going to use as follicles
                seeds = IECore.UniformRandomPointDistributionOp()( mesh=mesh, numPoints=args["count"].value )
                
                ## The evaluator needs a triangulated mesh
                meshTri = IECore.TriangulateOp()( input=mesh, throwExceptions=False )
                
                meshEvaluator = IECore.MeshPrimitiveEvaluator( meshTri )
                result = meshEvaluator.createResult()
        
                ## These are going to hold the data for our CurvesPrimtive
                p = IECore.V3fVectorData()
                vpc = IECore.IntVectorData()
                
                ## We don't want the geometric normal in result.normal(), so we find this in advance.
                normalsPrimVar = meshTri["N"]
                                
                ## Work out the CVs for a hair for each point, and push them onto the p data array.
                for i in range( seeds.numPoints ):
                        meshEvaluator.closestPoint( seeds["P"].data[i], result ) 
                        numCvs = self.__hairCVs( result.point(), result.vectorPrimVar( normalsPrimVar ), args, p )
                        vpc.append( numCvs )
                        
                curves = IECore.CurvesPrimitive( vpc, IECore.CubicBasisf.linear(), False, p )
                
                ## Apply any modifiers we may have
						
                ## This returns a tuple of tuples in the form
                ##    ( classInstance, parameterName, className, classVersion )
                modifiers = self.parameters()["hair"]["modifiers"].getClasses( True )
                for m in modifiers :
                
                    if m[0] and hasattr( m[0], "doModify" ) :
                     
                    	## We pass the appropriate part of the args dict down
                    	## to allow overrides etc...
                    	m[0].doModify( curves, args["modifiers"][ m[1] ] )					
							
                ## We need vTangents for the Marschner shading model.
                curves = IECore.CurveTangentsOp()( input=curves )
                                
                return curves


        def __hairCVs( self, origin, direction, args, pointsData ):
						
                segments = 10
                segLength = args["length"].value/float(segments+1)
                
                growth = direction.normalized()
                                
                pointsData.append( origin )
                
                lastP = origin
                for i in range(segments+1):
                        
                        thisP = lastP + growth * segLength
                        lastP = thisP
                 
                        pointsData.append( thisP )
                
                return segments + 2

```

# Setup #

You can create an instance of this procedural by pasting the above into the Maya Script Editor, and running:

```
import IECoreMaya
import maya.cmds

p = maya.cmds.createNode( "ieProceduralHolder" )
fp = IECoreMaya.FnProceduralHolder( p )
fp.setParameterised( ShadedHairExample() )
```

Create a Maya Point light, and turn on lighting with the **7** key.

You can also connect the **wordMesh** output of any mesh of your choosing to the
**parm\_mesh** attribute on the procedural, to grow hair on things other than the very exciting plane.

# Modifiers #

This example also demonstrates the use of a \see IECore.ClassVectorParameter, and the \see IECore.ClassLoader to maintain a list of 'modifiers' that can be added at will. This example provides basic 'frizz' and 'gravity' modifiers.

You may notice in the "modifiers" parameter definition in the code, it tells the ClassLoader maintained by the parameter to use the following search path:

**HAIR\_MODIFIER\_PATHS**

In order to experiment with this feature, you need to make a directory somewhere on your file system, and set this envVar to point to it.
In tcsh, this is done with:

> setenv HAIR\_MODIFIER\_PATHS /test/hairExample/modifiers

Then create the following structure inside this directory:

  * ./frizz
  * ./frizz/frizz-1.py:

```
import IECore
import random

class frizz( IECore.Parameterised ) :
	
	def __init__( self ) :
		
		IECore.Parameterised.__init__( self, "" )
		
		self.parameters().addParameters(
		
			[
		
				IECore.BoolParameter(
					name = "active",
					description = "Turns this component on and off.",
					defaultValue = True,
					userData = {
						"UI" : {
							"classVectorParameterPreHeader" : IECore.BoolData( True ),
							"visible" : IECore.BoolData( False ),
						}
					}
				),
		
				IECore.StringParameter(
					name = "label",
					description = "A descriptive label for the component. This has no effect whatsoever on the shading but can be useful "
						"for organisational purposes.",
					defaultValue = "Frizz",
					userData = {
						"UI" : {
							"visible" : IECore.BoolData( False ),
						},
					},
				),

				IECore.FloatParameter(
					name = "amount",
					description = "How much to randomise each vertex.",
					defaultValue = 1.0,
					userData = {
						"UI" : {
							"classVectorParameterHeader" : IECore.BoolData( True ),
						},
					},
				)
			]
		)

	def doModify( self, curves, args ) :
		
		if not args["active"].value :
			return
		
		## Make sure our frizz will be based on the same sequence of random numbers, though
		## we aren't necessarily sure the follicle seeds will be in the same order...
		random.seed( 0 )
		
		points = curves["P"].data
		vpc = curves.verticesPerCurve()
		amount = args["amount"].value
		
		i = 0
		for c in range( curves.numCurves() ) :
			
			v = 0;
			vStep = 1.0 / vpc[c]
			
			for v in range( vpc[c] ) :
				
				localAmount = amount * v * vStep							
				
				points[i] += IECore.V3f( random.random()-0.5, random.random()-0.5, random.random()-0.5 ) * localAmount
				
				i += 1
```

  * ./gravity
  * ./gravity/gravity-1.py:

```
import IECore
import random

## Very basic pretend gravity. 
class gravity( IECore.Parameterised ) :
	
	def __init__( self ) :
		
		IECore.Parameterised.__init__( self, "" )
		
		self.parameters().addParameters(
		
			[
		
				IECore.BoolParameter(
					name = "active",
					description = "Turns this component on and off.",
					defaultValue = True,
					userData = {
						"UI" : {
							"classVectorParameterPreHeader" : IECore.BoolData( True ),
							"visible" : IECore.BoolData( False ),
						}
					}
				),
		
				IECore.StringParameter(
					name = "label",
					description = "A descriptive label for the component. This has no effect whatsoever on the shading but can be useful "
						"for organisational purposes.",
					defaultValue = "Gravity",
					userData = {
						"UI" : {
							"visible" : IECore.BoolData( False ),
						},
					},
				),

				IECore.FloatParameter(
					name = "amount",
					description = "How much to attract each vertex downwards in object space.",
					defaultValue = 1.0,
					userData = {
						"UI" : {
							"classVectorParameterHeader" : IECore.BoolData( True ),
						},
					},
				)
			
			]
		)

	def doModify( self, curves, args ) :
		
		if not args["active"].value :
			return
		
		points = curves["P"].data
		vpc = curves.verticesPerCurve()
		amount = args["amount"].value

		i = 0
		for c in range( curves.numCurves() ) :
			
			v = 0;
			vStep = 1.0 / vpc[c]
		
			for v in range( vpc[c] ) :
				
				localAmount = amount * v * vStep
				points[i] += IECore.V3f( 0, -(localAmount*localAmount), 0 )
				i += 1
```

Open the "modifiers" section of the "hair" parameters. If you click on the grey plus sign, you should then be able to create instances of these basic modifiers.

Click on the layer icon for more options.

If you want to experiment writing your own, you simply need the

```
className/className-version.py
```

structure witin the HAIR\_MODIFIER\_PATHS for the ClassLoader to pick up your new classes. This is cached, so if you dont see them right away, try restarting maya.