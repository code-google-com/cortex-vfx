The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary intro to procedurals
#sidebar ExamplesTOC

# What is a Procedural? #
A procedural is an Operator that can visualize itself in different contexts (for example OpenGl or RenderMan). It can be completely platform independent and produce the same output in various host applications like Maya, Houdini or even you own home baked system.

Procedurals can be written in either Python or C++, but for this introduction we will just look at the python implementation.

# Installing Procedurals #

The trick to getting procedurals to render seamlessly is where they're installed. The environment variable which Cortex's ClassLoader class uses to find and Procedurals is:

`$IECORE_PROCEDURAL_PATHS`

This is a colon-delimited path list and under each path the Procedurals must be explicitly versioned and installed in it's own directory according to:

`<PATH>/<PROCEDURAL>/<PROCEDURAL>-<VERSION>.py`

For example:
```
myProcedurals/
|-- cubeProcedural
|   `-- cubeProcedural-1.py
|-- renderPointCloud
|   |-- renderPointCloud-1.py
|   `-- renderPointCloud-2.py
`-- sphereProcedural
    |-- sphereProcedural-1.py
    |-- sphereProcedural-2.py
    `-- sphereProcedural-3.py
```

Nesting is also possible:

```
myProcedurals/
|-- categoryA
|   `-- procA
|       `-- procA-1.py
|   `-- procB
|       `-- procB-1.py
|-- categoryX
|   `-- procM
|       `-- procM-1.py
|   `-- procN
|       `-- procN-1.py

```

# Rendering Procedurals #
## Maya ##
Make sure you have the **ieCore plugin** loaded. You can instantiate an **ieProceduralHolder** node and initialise it with a procedural using the following python. In this example we use _sphereProcedural-1_.
```
import IECoreMaya
IECoreMaya.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 )
```
![http://cortex-vfx.googlecode.com/svn/wiki/images/SimpleProceduralExample.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/SimpleProceduralExample.jpg)
## Houdini ##
Like maya, you can instantiate an **ieProceduralHolder** (a SOP in this case) and initialise it with a procedural using the following python.
```
import IECoreHoudini
IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 )
```
Or you can just create it from the node tab-menu in the sop context.

![http://cortex-vfx.googlecode.com/svn/wiki/images/CortexHoudini_createsphere.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/CortexHoudini_createsphere.jpg)

## 3Delight ##

Rendering requires the procedural and it's parameters being injected into the rib. For the above example that would be something like this:
```
ArchiveBegin "|cortex_sphere|cortex_sphereShape"  # {
  # |cortex_sphere|cortex_sphereShape

  AttributeBegin  # {
    Procedural "DynamicLoad" [ "iePython" "IECoreRI.executeProcedural( \"sphereProcedural\", 1, ['-radius', '1', '-theta', '360' ] )" ] [ -1 1 -1 1 -1 1 ] 
  AttributeEnd  # }

ArchiveEnd  # }

```

The above produces this beautiful sphere

![http://cortex-vfx.googlecode.com/svn/wiki/images/Cortex_render_01.png](http://cortex-vfx.googlecode.com/svn/wiki/images/Cortex_render_01.png)

There are multiple ways of getting the above entry into your rib:

  * **Houdini** - In order to inject `ieProceduralHolder` SOP nodes from Houdini we need to attach a `cortexRmanInject` geometry shader to our procedural geometry. This will then inject the procedural and it's parameters into the RIB as required, consult the **Cortex & Houdini** Examples
  * **Maya** - If you render from Maya using 3DelightForMaya (3DFM) then `ieProceduralHolder` nodes will get injected into the RIB like this automatically.