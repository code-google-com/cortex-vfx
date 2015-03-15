The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Houdini application support for Cortex.
#sidebar IECoreHoudini\_TOC

# Introduction #

**IECoreHoudini** provides an interface for working with **Cortex** within SideFX's **Houdini**.

  * Parameterised Procedurals evaluation & preview in SOPs.
  * Interactive pick & transform.
  * OpenGL Material preview.
  * Converter for translating SOP geometry into Cortex primitives.
  * Utility HDAs for working with Cortex in Houdini.

**NOTE:** this is an early release of code. You have been warned.

# Building/Installation #

## Dependencies ##

First, make sure you have a working installation of **Cortex v5** from the **trunk**. You will need at least **[r3393](https://code.google.com/p/cortex-vfx/source/detail?r=3393)**. It must be built against **Python 2.5** and **Boost 1.33.1** to work correctly with **Houdini 10.0**.

## Building with SCons ##

You can build using:
```
% scons OPTIONS=/path/to/my/options
```

Depending on your setup, your options file may need to specify :
  * `HOUDINI_ROOT`- This is your houdini installation path. This defaults to the `$HFS` envvar.
  * `HOUDINI_CXX_FLAGS` - The cflags used by Houdini to build dsos (see below).
  * `INSTALL_HOUDINILIB_NAME` - The name under which to install the Houdini libraries.
  * `INSTALL_HOUDINIPLUGIN_NAME` - The name under which to install Houdini plugins.
  * `INSTALL_HOUDINIOTL_DIR` - The directory in which to install Houdini otls.
  * `INSTALL_HOUDINIICON_DIR` - The directory under which to install Houdini icons.

The scons install target will install the Houdini components into suitable locations. By default these are under _$CORTEX/houdini_ but can be overridden using the _INSTALL_ variables described above.

## Houdini CFlags ##

Houdini passes a lot of flags to the compiler when building dsos. The easiest way to find these out is to do a dummy build with Houdini's _hcustom_ tool.

```
% touch test.cpp
% hcustom -t -e -i /tmp test.cpp
g++ -DVERSION=\"10.0.554\" -DDLLEXPORT= -D_GNU_SOURCE -DLINUX -DAMD64 -m64 -fPIC 
-DSIZEOF_VOID_P=8 -DSESI_LITTLE_ENDIAN -DENABLE_THREADS -DUSE_PTHREADS -D_REENTRANT 
-D_FILE_OFFSET_BITS=64 -c -DGCC4 -DGCC3 -Wno-deprecated -I/drd/software/ext/houdini/lin64
/hfs10.0.554/toolkit/include -I/drd/software/ext/houdini/lin64/hfs10.0.554/toolkit/include
/htools -Wall -W -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wno-uninitialized 
-Wunused -Wno-unused-parameter -O2 -DMAKING_DSO -o test.o test.cpp
```

## Testing ##

Run the unit tests from the **IECoreHoudini** root using Houdini's _hython_ command-line interpreter.
```
 % hython test/All.py
```
**Note:** some of the tests may print "Render failed." when executing. This is a known problem with executing the render callback outside the gui and not a test fail.

# Procedurals #

**IECoreHoudini** defines a SOP node for holding Procedurals. This is under the tab menu as **Cortex Procedural** (node type is **ieProceduralHolder**).

By default, when a **Cortex Procedural** is dropped it has no parameterised procedural associated with it.

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_operror-e1274145048691.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_operror-e1274145048691.jpg)

<sup>A procedural holder SOP in error because it doesn't have a parameterised procedural associated with it.</sup>

You can pick a procedural from the drop-down on the 'Class' parameter tab to associate it with this node.

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/06/IECoreHoudini_pamInterface-e1277430114264.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/06/IECoreHoudini_pamInterface-e1277430114264.jpg)

<sup>Picking a procedural/version from the interface.</sup>

When a parameterised procedural is associated with the SOP it will be rendered in the OpenGL viewport and it's parameters will appear on the **Parameters** pane allowing you to interface with the procedural.

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_grab1-e1274144990789.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_grab1-e1274144990789.jpg)

<sup>A parameterised procedural with parameters, previewed in the Houdini viewport.</sup>

We can also create a parameterised procedural and associate it with a **Cortex Procedural** via Python, using the `IECoreHoudini.FnProceduralHolder()` function set, just like in **IECoreMaya**. The function set simply takes a [hou.Node()](http://www.sidefx.com/docs/houdini10.0/hom/hou/Node) instance at initialisation to specify which SOP to operate on.

In this example we create an instance of the **sphereProcedural** procedural and assign it to an **ieProceduralHolder** SOP.

```
import IECore, IECoreHoudini
geo = hou.node("/obj").createNode("geo", run_init_scripts=False)
cortex_proc = geo.createNode( "ieProceduralHolder" )
some_procedural = IECore.ClassLoader.defaultProceduralLoader().load( "sphereProcedural", 1 )()
fn = IECoreHoudini.FnProceduralHolder( cortex_proc )
fn.setParameterised( some_procedural )
```

There is also a short-hand for this...
```
import IECoreHoudini
IECoreHoudini.FnProceduralHolder.create("cortex_sphere", "sphereProcedural", 1 )
```

You can interact with the procedural geometry in exactly the same way you would a normal Houdini object.

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_interactive-e1274145129841.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_interactive-e1274145129841.jpg)

<sup>These are not your everyday spheres - each of them is a separate geometry object containing a ieProceduralHolder node which is rendering the example sphereProcedural - but you can pick and transform them as usual.</sup>

OpenGL shading and lighting works as expected.

**NOTE:** there is a known limitation currently, that the material is only applied to the first render primitive rendered by a procedural.

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_material-e1274145409795.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_material-e1274145409795.jpg)

<sup>A Cortex procedural (half-dome) with a Decal material applied and a point light.</sup>

## Parameters ##

The dynamic parameter interface is created when a procedural is set on the holder SOP and can represent these standard parameter types:
  * `Int `
  * `Float`
  * `Double`
  * `Bool`
  * `String`
  * `Path`, `DirName`, `FileName`, `FileSequence`
  * `V2i`, `V2f`, `V2d`
  * `V3i`, `V3f`, `V3d`
  * `Color3f`, `Color4f`
  * `M44f`, `M44d`
  * `Box2i`, `Box2f`, `Box2d`
  * `Box3i`, `Box3f`, `Box3d`

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/06/IECoreHoudini_parameterPane-e1277430168351.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/06/IECoreHoudini_parameterPane-e1277430168351.jpg)

Currently unsupported parameter types are:
  * `VectorParameter` types
  * `Compound`
  * `FrameList`

**NOTE:** Presets are also not currently supported.

**IECoreHoudini** can modify behaviour/appearance based on some optional parameter _userData_ entries.

  * **label** (`StringData`) - the parameter label in Houdini. Defaults to using a pretty-formatted version of the parameter's name.
  * **gui\_update** (`BoolData`) - whether this parameter will force a gui update. Defaults to True.
  * **hidden** (`BoolData`) - this can be used to hide a parameter from the interface. the parameter still exists and evaluates as usual but is 'invisible' in Houdini terms. Defaults to False.

For example, here is a `V3iParameter` with a label which will not update the gui when it is modified:
```
V3iParameter(
     name = "v3iparm",
    description = "A v3i parameter",
    defaultValue = V3iData( V3i( 5, 5, 5 ) ),
    userData = {
        "label":StringData("My V3i Parameter"),
        "gui_update":BoolData(False)
    }
)
```

# Conversions #

The `IECoreHoudini.FromHoudiniSopConverter()` class can convert native Houdini SOP geometry (meshes or points) into Cortex Primitives. Meshes and points are converted to their Cortex equivalent (PointsPrimitive or MeshPrimitive) and attributes are converted into Cortex primitive variables.

This example takes a Scatter SOP (_/obj/geo1/scatter_) and using the converter creates a Cortex `IECore.PointsPrimitive` object.

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_converter-e1274145524449.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_converter-e1274145524449.jpg)

```
import IECoreHoudini
scatter = hou.node( "/obj/geo1/scatter1" )
converter = IECoreHoudini.FromHoudiniSopConverter( scatter )
cortex_points = converter.convert()
```

# Utility HDAs #

There are a couple of digital assets included with **IECoreHoudini** to make life a little bit easier.

## Cortex Rman Inject ##
This is a geometry SHOP for injecting a Cortex Procedural into a render via Soho's RIB generation. It wraps up the native Renderman Delayed Load geometry SHOP and adds some extra controls for specifying alternative bounds and adding extra procedural arguments.

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_render-e1274145571327.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_render-e1274145571327.jpg)

## Cortex Writer ##
This is a SOP geometry writer ROP which converts SOP geometry and writes out FIO or COB files. Behind the scenes it just uses a regular geometryROP and an instance of `IECoreHoudini.FromHoudiniSopConverter()` that calls `convert()` per frame before writing the output. Feel free to dive in and have a look!

![http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_writer-e1274145550854.jpg](http://www.pawfal.org/dan/blog/wp-content/uploads/2010/05/cortexHoudini_writer-e1274145550854.jpg)