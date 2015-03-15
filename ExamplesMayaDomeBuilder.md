The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Building a maya light dome from an environment map
#sidebar ExamplesTOC

## Introduction ##

This code example creates a dome of spotlights in maya, distributed according to the intensities in an environment map. It demonstrates the use of Ops, the loading of images, the sampling of an environment map using the median cut algorithm and the automatic creation of maya user interfaces from a set of parameters.

![http://opensource.image-engine.com/images/examples/lightDome.png](http://opensource.image-engine.com/images/examples/lightDome.png)

## Code ##

Either cut and paste the code below directly into the maya script editor, or save it to a file and run it with execfile(). When executed it pops up a window which controls the creation of the dome.

```
import IECore
import IECoreMaya
import maya.cmds

# The DomeBuilder derives from the Op class, which is a base class for things which
# perform an operation based on the values of some input Parameters.
class DomeBuilder( IECore.Op ) :

        def __init__( self ) :
        
                # The base class is initialised with the name for this Op, a description of
                # what it does, and a Parameter describing what the result will be.
                IECore.Op.__init__(
                
                        self,
                        "Builds light domes from environment maps.",
                        IECoreMaya.DAGPathParameter(
                                name = "result",
                                description = "The name of the top level transform of the dome.",
                                defaultValue = "",
                                allowEmptyString = True,
                        )
                
                )
                
                # We then add the Parameters which will control how the Op behaves. Parameters have
                # a name, description, defaultValue and possibly a list of preset values.
                self.parameters().addParameters(
                
                        [
                        
                                IECore.FileNameParameter(
                                        name = "environmentMap",
                                        description = "The environment map from which to create the dome.",
                                        defaultValue = "",
                                        check = IECore.FileNameParameter.CheckType.MustExist,
                                        allowEmptyString = False,
                                ),
                                
                                IECore.V3fParameter(
                                        name = "centre",
                                        description = "The centre of the dome in world space.",
                                        defaultValue = IECore.V3f( 0 ),
                                ),
                                
                                IECore.FloatParameter(
                                        name = "scale",
                                        description = "The scale of the dome.",
                                        defaultValue = 1,
                                ),
                                
                                IECore.V3fParameter(
                                        name = "rotation",
                                        description = "The rotation of the dome in world space.",
                                        defaultValue = IECore.V3f( 0 ),
                                ),

                                IECore.IntParameter(
                                        name = "lights",
                                        description = "How many lights will be created.",
                                        defaultValue = 6,
                                        presets = (
                                                ( "8", 3 ),
                                                ( "16", 4 ),
                                                ( "32", 5 ),
                                                ( "64", 6 ),
                                                ( "128", 7 ),
                                                ( "256", 8 ),
                                        ),
                                        presetsOnly = True
                                ),
                
                        ]
                        
                )

        # All ops must implement the doOperation() method. This is passed the values for each parameter
        # and must return the result.
        def doOperation( self, args ) :
                
                # load the image. we don't need to know what sort of file it is as the Reader will create a
                # suitable Reader for us automatically.
                reader = IECore.Reader.create( args["environmentMap"].value )
                if not reader :
                        raise IOError( "Unable to create reader for file \"" + args["environmentMap"].value + "\"" )
                                
                image = reader.read()
                
                # check we have an RGB image and convert all channels to float.
                for n in [ "R", "G", "B" ] :
                        
                        if not n in image :
                                raise Exception( "Image doesn't have RGB channels." )
                                
                        p = image[n]
                        p.data = IECore.DataCastOp()( object=image[n].data, targetType=IECore.FloatVectorData.staticTypeId() )
                        image[n] = p
                        
                # run the sampling op. this is implemented in C++ within IECore and generates a set of directions and colors for each light.
                lights = IECore.EnvMapSampler()( image=image, subdivisionDepth=args["lights"].value )

                # create the lights in maya using the information we got from the EnvMapSampler
                lightParentNames = []
                for i in range( 0, len( lights["directions"] ) ) :
                
                        # make the light
                        lightName = maya.cmds.spotLight()
                        lightParentName = maya.cmds.listRelatives( lightName, parent=True, path=True )[0]
                        
                        # get the colour right - people like bright colours and extreme intensities rather than 
                        # colours so bright or dark you can't see 'em. the 6.66 multiplier was chosen empirically to make
                        # sure that a 100% diffuse object would have a value of 1 when lit with a dome generated from a 100% white
                        # constant map. i'm sure it's just a coincidence that it's the number of the beast.
                        color = lights["colors"][i] * 6.66
                        brightest = max( color[0], color[1], color[2] )
                        color /= brightest
                        maya.cmds.setAttr( lightName + ".color", color[0], color[1], color[2] )
                        maya.cmds.setAttr( lightName + ".intensity", brightest )
                        
                        # get the transform right
                        transform = IECore.M44f.createAimed( IECore.V3f( 0, 0, -1 ), lights["directions"][i], IECore.V3f( 0, 1, 0 ) )
                        s, h, r, t = transform.extractSHRT()
                        r = IECore.radiansToDegrees( r )
                        maya.cmds.xform( lightParentName, rotation=( r[0], r[1], r[2] ) )
                        maya.cmds.xform( lightParentName, translation=( 0, 0, 4 * args["scale"].value ), objectSpace = True )
                        maya.cmds.xform( lightParentName, rotatePivot=( 0, 0, 0 ), worldSpace = True )
                        maya.cmds.setAttr( lightParentName + ".scale", lock = True )
                        
                        # add to the list
                        lightParentNames.append( lightParentName )
                        
                # group and position them together
                group = maya.cmds.group( lightParentNames, name="domeLights", world=True )
                if group[0]!="|" :
                        # seems like maya isn't always returning unique names here. but we know we've parented it in
                        # the root of the world so this should make it unique.
                        group = "|" + group
                        
                maya.cmds.xform( group, objectSpace=True, pivots=( 0, 0, 0 ) )
                
                centre = args["centre"].value
                rotation = args["rotation"].value
                maya.cmds.xform( group, translation=( centre[0], centre[1], centre[2] ), rotation=( rotation[0], rotation[1], rotation[2] ) )
                                
                return IECore.StringData( group )

# This line opens a window containing the ui for a dome builder - the interface is created automatically based on the information in
# the Parameters.               
IECoreMaya.OpWindow( DomeBuilder() )
```