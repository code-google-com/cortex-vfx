The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Simple Op to load a mesh
#sidebar ExamplesTOC

# Simple Op to Load a Mesh #

This simple Op loads a MeshPrimitive from a file. You could use it right from python if you wanted, but it is most useful in host applications on an op holder. This would allow you to load a cob in Maya, for example, by connecting the result plug of the op holder to the inMesh plug of a Maya mesh shape. The same Op can be used in Houdini by simply wiring the CortexOp SOP output into the CortexToHoudini SOP input.

# Code #
```
import IECore

class meshReader( IECore.Op ) :

	def __init__( self ) :
	
		IECore.Op.__init__( self, "",
			IECore.MeshPrimitiveParameter(
				"result",
				"The mesh",
				IECore.MeshPrimitive()
			)
		)

		self.parameters().addParameter(
			IECore.FileNameParameter(
				"fileName",
				"The file to load",
				""
			)
		)

	def doOperation( self, args ) :
		
		return IECore.ObjectReader( args["fileName"].value ).read()

IECore.registerRunTimeTyped( meshReader )
```