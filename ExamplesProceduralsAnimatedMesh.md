The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Rendering animated meshes using a procedural
#sidebar ExamplesTOC

This example pulls together the ExamplesMayaMeshExport and the ExamplesMayaVertCache by using the resulting files to render animation from within a procedural.

## The Procedural ##

This code implements the procedural. It should be saved in a file called vertCacheProcedural.py.

```

from __future__ import with_statement

import os
import glob

import IECore

class vertCacheProcedural( IECore.ParameterisedProcedural ) :

        ## In the constructor we add all the parameters which define how this procedural works.
        def __init__( self ) :
        
                IECore.ParameterisedProcedural.__init__( self, "Renders cob files with vertex cached animation." )
                
                self.parameters().addParameters(
                
                        [
                
                                IECore.DirNameParameter(
                                        name = "objectDirectory",
                                        description = "A directory holding the objects to be rendered.",
                                        defaultValue = "",
                                        allowEmptyString = True,
                                        check = IECore.DirNameParameter.CheckType.MustExist,
                                ),
                
                                IECore.FileSequenceParameter(
                                        name = "animationSequence",
                                        description = "A file sequence of animation caches to be applied. "
                                                "Use # characters to specify the frame number.",
                                        defaultValue = "",
                                        allowEmptyString = True,
                                ),
                                
                                IECore.FloatParameter(
                                        name = "frame",
                                        description = "The frame number of the cache to render.",
                                        defaultValue = 1,
                                        userData = IECore.CompoundObject( {
                                                "maya" : {
                                                        "defaultConnection" : IECore.StringData( "time1.outTime" ),
                                                },
                                        } ),
                                
                                ),
                
                        ]
                
                )
                
                self.__lastAnimationCache = None
                self.__lastAnimationSequence = ""
        
        # This method must be implemented to return the bounding box of everything that
        # will be rendered. The idea is that this method should be fast, so that if the
        # bounding box isn't seen by the camera, the renderer can quickly discard the procedural
        # without expanding it. Even if the bounding box is on screen, the renderer can still
        # delay opening it till the last minute 
        def doBound( self, args ) :
        
                animationCache = self.__animationCache( args )
                        
                result = IECore.Box3f()
                files = self.__fileNames( args )
                for f in files :
                
                        objectBound = None
                        if animationCache :
                                
                                objName = os.path.splitext( os.path.split( f )[1] )[0]
                                try :   
                                        objectBound = animationCache.read( args["frame"].value, objName, "vertCache.boundingBox" ).value
                                except :
                                        IECore.msg( IECore.Msg.Level.Error, "vertCacheProcedural::doBound", "Cache contains no bound for object \"%s\"." % objName )
                                        
                        else :
                        
                                o = self.__loadFile( f )
                                objectBound = o.bound()
                                
                        if objectBound :
                                result.extendBy( objectBound )

                return result

        # This method can be used to set renderer attributes outside of the procedural.
        # This can sometimes be necessary to let the renderer know when it needs to
        # open the procedural - for instance it might set raytrace visibility attributes.
        def doRenderState( self, renderer, args ) :
        
                pass

        # This method is called when the renderer wants to know what exists inside the
        # bounding box returned by doBound. Here we load the geometry, deform it, and
        # pass it to the renderer.
        def doRender( self, renderer, args ) :
        
                animationCache = self.__animationCache( args )
                        
                files = self.__fileNames( args )
                for f in files :
                
                        objName = os.path.splitext( os.path.split( f )[1] )[0]
                        
                        o = self.__loadFile( f )
                        if animationCache :
                                
                                p = None
                                try :
                                        p = animationCache.read( args["frame"].value, objName, "vertCache.P" )
                                except :
                                        IECore.msg( IECore.Msg.Level.Error, "vertCacheProcedural::doRender", "Cache contains no points for object \"%s\"." % objName )
                                
                                if p :
                                        
                                        if len( p )==o.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) :
                                                o["P"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, p )
                                                if o.isInstanceOf( IECore.MeshPrimitive.staticTypeId() ) and o.interpolation=="linear" and "N" in o :
                                                        IECore.MeshNormalsOp()( copyInput=False, input=o )
                                        else :
                                                IECore.msg( IECore.Msg.Level.Error, "vertCacheProcedural::doRender", "Vertex cache for object \"%s\" has wrong number of points" % objName )
                                                
                        with IECore.AttributeBlock( renderer ) :
                        
                                renderer.setAttribute( "name", IECore.StringData( objName ) )
                                
                                o.render( renderer )
                                                
        def __fileNames( self, args ) :
        
                return glob.glob( args["objectDirectory"].value + "/*.cob" )

        def __loadFile( self, fileName ) :
        
                # By using a CachedReader to load a file we reduce network overhead when
                # the same file must be loaded repeatedly. This helps boost interactivity
                # in maya.
                return IECore.CachedReader.defaultCachedReader().read( fileName )

        def __animationCache( self, args ) :

                sequence = args["animationSequence"].value
                if sequence=="" :
                        return None
                
                if sequence==self.__lastAnimationSequence :
                        return self.__lastAnimationCache
                        
                self.__lastAnimationCache = IECore.InterpolatedCache( sequence, IECore.InterpolatedCache.Interpolation.Linear, IECore.OversamplesCalculator( 24.0, 1 ) )
                return self.__lastAnimationCache
            
IECore.registerRunTimeTyped( vertCacheProcedural )
	
```

## Using the procedural ##

You can see test the procedural in maya quite easily using the following code to create a node to hold it.

```
execfile( "/path/to/vertCacheProcedural.py" )

import IECoreMaya
import maya.cmds

p = maya.cmds.createNode( "ieProceduralHolder" )
fp = IECoreMaya.FnProceduralHolder( "ieProceduralHolder1" )
fp.setParameterised( vertCacheProcedural() )
```

The Parameters section of the attribute editor then allows you to specify the meshes and caches you want to render. Here we're using the meshes and caches exported by the MeshExportExample and AnimationExportExample.


![http://opensource.image-engine.com/images/examples/animatedProcedural.png](http://opensource.image-engine.com/images/examples/animatedProcedural.png)