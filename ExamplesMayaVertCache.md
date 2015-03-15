The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Exporting maya animation as vertcaches
#sidebar ExamplesTOC

This simple example demonstrates the export of animation to a vertex cache using cortex. It can be used to generate vertex caches which can be used with the  ExamplesProceduralsAnimatedMesh. It shows how to use the FileSequence and AttributeCache classes as well as how to convert maya objects to cortex objects.

```
import os

import maya.cmds

import IECore
import IECoreMaya

def writeVertexCaches( meshes, destinationFileSequence ) :

	destinationDirectory = os.path.dirname( str( destinationFileSequence ) )
	if not os.path.exists( destinationDirectory ) :
		os.makedirs( destinationDirectory )
		
	for frameNumber in destinationFileSequence.frameList.asList() :
	
		maya.cmds.currentTime( frameNumber )
		
		attributeCache = IECore.AttributeCache( destinationFileSequence.fileNameForFrame( frameNumber * 250 ), IECore.IndexedIOOpenMode.Write )
		
		for mesh in meshes :
		
			converter = IECoreMaya.FromMayaShapeConverter.create( mesh )
			converter["space"].setValue( "World" )
			points = converter.points()
			
			bound = IECore.PointBoundsOp()( points=points )
			attributeCache.write( mesh, "vertCache.P", points )
			attributeCache.write( mesh, "vertCache.boundingBox", bound )

allMeshes = maya.cmds.ls( selection=True, noIntermediate=True, dag=True, type="mesh" )
writeVertexCaches( allMeshes, IECore.FileSequence( "/tmp/myCaches/cache.######.fio", IECore.FrameRange( 1, 50 ) ) )



```