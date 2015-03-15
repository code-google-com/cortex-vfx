The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Surfacing particles using marching cubes and an implicit surface function
#sidebar ExamplesTOC

This example fills a mesh primitive with particles, then creates a new mesh primitive by surfacing those particles using marching cubes and an implicit surface function.

In production, particle caches are read from disk using a Particle Reader and the resulting meshes are used for rendering the particle surface.

As of Cortex 4.1.0 there are particle readers for Cortex Points Primitives (.cob), Maya (.pdc), Renderman (.ptc), Maya nParticles (.mc), and Houdini (.bgeo).

```
import os

import IECore


def createParticleSurface( particles, smoothingRadius, marchResolution, marchThreshold ) :
	
	isoFn = IECore.ZhuBridsonImplicitSurfaceFunctionV3dd(
		particles['P'].data,
		particles['radius'].data,
		smoothingRadius
	)
	
	bound = IECore.Box3d( particles['boundMin'].data.value, particles['boundMax'].data.value )
	
	builder = IECore.MeshPrimitiveBuilder()
	marcher = IECore.MarchingCubesd( isoFn, builder )
	marcher.march( bound, marchResolution, marchThreshold )
	mesh = builder.mesh()
	
	return mesh

def fillMeshWithParticles( meshFile, numParticles ) :
	
	mesh = IECore.Reader.create( meshFile ).read()
	mesh = IECore.TriangulateOp()( input=mesh, throwExceptions=False )
	meshEvaluator = IECore.MeshPrimitiveEvaluator( mesh )
	
	bound = mesh.bound()
	boundMin = bound.min
	boundMax = bound.max
	boundSize = bound.size()
	
	positions = IECore.V3dVectorData()
	radii = IECore.DoubleVectorData()
	particleBound = IECore.Box3d()
	
	random = IECore.Rand32()
	
	while len(positions) < numParticles :
		
		p = random.nextV3f() * boundSize + boundMin
		d = meshEvaluator.signedDistance( p )
		
		if d < 0.0 :
			p = IECore.V3d( p )
			positions.append( p )
			particleBound.extendBy( p )
			radii.append( 1.0 )
	
	particles = IECore.PointsPrimitive( len(positions) )
	particles['P'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, positions )
	particles['radius'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, radii )
	particles['boundMin'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3d( boundMin ) )
	particles['boundMax'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3d( boundMax ) )
	
	return particles

particleFile = '/tmp/myParticles.cob'

if os.path.exists( particleFile ) :
	particles = IECore.Reader.create( particleFile ).read()
else :
	particles = fillMeshWithParticles( "human.cob", 100000 )
	IECore.Writer.create( particles, particleFile ).write()

smoothing = 0.75
resolution = IECore.V3i( 200, 300, 200 )
threshold = 0.9

mesh = createParticleSurface( particles, smoothing, resolution, threshold )
writer = IECore.ObjectWriter( mesh, '/tmp/myMesh.cob' )
writer.write()
```