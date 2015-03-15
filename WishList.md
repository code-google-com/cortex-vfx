The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Things we might want to aim for in future versions

## SeExpr support ##

It would be great to be have a simple SeExpr wrapper in cortex, making it super easy to apply SeExpr to VectorData. I wonder if it might be even better to have generic python bindings for SeExpr, and make VectorData support a buffer protocol that allows it to work with it? That way cortex doesn't get yet another dependency, and we get to use the buffer protocol stuff with other things.

## IO improvements ##

FileIndexedIO has less than ideal performance when opening very large files, as it reads the whole index at open time. This somewhat negates any advantages subsequent random access gives. It'd be great to address this, either with optimisations to FileIndexedIO or with a new backend perhaps based around HDF5. We also need to do a much better job of validating names for "files" - we keep accidentally having slashes in them.

## Data improvements ##

We might like to introduce some sort of native support for IndexedData.

It might also be interesting to provide simple methods for getting threadsafe access to TypedData::readable() and TypedData::writable(). These could allow multiple concurrent readers but only a single writer.

We might like to introduce specialized PointVectorData, NormalVectorData, and VectorVectorData (although the last name is terribly confusing...) to distinguish between the various data that is all currently stored as V3fVectorData.

## Parameter improvements ##

It'd be very useful to have a standard way of knowing when a Parameter value has changed. We currently do this manually using the slightly ugly updateCount mechanism which is only implemented in IECoreMaya. It might be great to have a Parameter::hash() method which gives a (kindof) unique hash for the current value. How we would quickly come up with reasonable hashes for very large Objects like MeshPrimitives could be a tricky question however. Perhaps there might be a good case for it being the responsibility of the host to compute the hash?

We could do with being able to serialise parameters in python using repr().

## Op improvements ##

It'd be great to be able to pass arguments to Op::operate() (and similar methods elsewhere), so that a single Op instance could easily be used with multiple sets of arguments, each in a different thread. This is motivated by the todo in IECore/CachedReader.cpp, and also earlier questions about the overhead of instantiating multiple identical ops in the same process.

Another particularly useful possibility this introduces could be the ability to pass LayeredDict and SubstitutedDict style argument lists to C++ ops. We currently do this for procedurals in python and it's invaluable but we hit problems when we need to pass such arguments to C++. We could maybe achieve this by having some sort of CompoundObjectInterface in C++, which defines a minimal API for getting parameter values.

One implication of this is that Ops really really have to stop cheating and getting their values from the parameters rather than from the operands passed to doOperation().

## Procedural improvements ##

We should revisit the ongoing argument we have internally about whether a renderer should be passed to Procedural::bound(). We have several procedurals which do change bounding box depending on whether they're rendered in GL or RenderMan, and it'd be nice to query this accurately. The argument against is that you should be able to always get a worst case bound - perhaps that is achieved by passing 0 for the renderer?

## IECoreHoudini improvements ##

It'd be great to move IECoreHoudini out of contrib and have it as a standard part of cortex. We might want to make minor adjustments so it conforms more closely to the cortex coding conventions, and also perhaps standardise a ParameterisedHolder api between IECoreMaya and IECoreHoudini.

## IECoreNuke improvements ##

It'd be great to see a ToCoreConverter which takes a Nuke node and returns an ImagePrimitive directly. Currently we render to temp files and then reread them which is a little ugly and inefficient.

## OIIO Support (carsten.kolve) ##
This is really more a discussion item than a soid aim. OIIO has made a lot of progress in the last year and now has support for nearly all of the relevant image formats, including exr, dpx and cin. If it was used as a backend to the ImagePrimitive, one could potentially benefit from a whole lot of active development on the project.
http://openimageio.org/

(hradec) I'm also a big fan of OIIO as part of cortex, specially because it's DDS support. As I'm using cortex on a game pipeline to generate heavy ray-traced data from 3delight, having DDS support would make things way easier on our end.

## Alembic Support (carsten.kolve) ##
Ok - this is probably even more premature (the first version is not even released). But in case it really does get the broad support from 3rd party developers that was promised, then it could be tempting to write reader/writer to and from common cortex primitives for interchange and compatibility purposes.

## CMake (carsten.kolve) ##
Switch to cmake to make the build process more transparent.

## Nuke Geometry (dan.bethell) ##

I'd like to see geometry converters for Nuke and an OpHolder equivalent. This would allow us to use geometry reading/workflow Ops in Nuke's 3d workflow.

## End User Tools ##

It'd be nice to have menus in Maya for creating procedurals, exporting geometry, creating and applying vertcaches etc. These probably wouldn't be used by the larger studios but might help smaller places and individuals start getting into cortex. IE needs to look into whether we can provide this or not - current agreement with management is that we open source libraries rather than end user tools.

## UI ##

soft min/max for relevant parameters would be great.

It'd be nice to be able to specify a uiLabel for a parameter, it's really usefull if you need to avoid language keywords in parameter names, or change something to make it more obvious/artist friendly, without breaking compatibility.

Another small thing, but if we could make the call signature for IECoreMaya.Class/ClassVectorParameterUI.registerClassMenuCallback match, so you don't need to write wrapper functions to re-use the code that does the work. At the moment its:
```
 callback( menuDefinition, parameter, holderNode ) # Class Parameter
 callback( menuDefinition, classVectorParameter, childParameter, holderNode ) # ClassVectorParameter
```
Which means you end up writing functions like
```
def myCallback( menuDefinition, classVectorParameter, childParameter, holderNode ) :
   ...
def myCallbackWrapper( menuDefinition, parameter, holderNode ) :
    myCallback( menuDefinition, None, parameter, holderNode )

IECoreMaya.ClassVectorParameterUI.registerClassMenuCallback( myCallback )
IECoreMaya.ClassParameterUI.registerClassMenuCallback( myCallbackWrapper )
```
It'd be nice to be able to write:
```
 myCallback( menuDefinition, parameter, holderNode, classVectorParameter=None ) :
    ...
IECoreMaya.ClassVectorParameterUI.registerClassMenuCallback( myCallback )
IECoreMaya.ClassParameterUI.registerClassMenuCallback( myCallback )
```
... as its pretty easy to handle Cs & CVs in one function.

It'd also be nice to be able to specify exclusive conditions to the ClassLoader to allow specific classes/matched classes to be hidden.

It'd be good to separate out the 'label' from the 'control' in all parameter UIs. There are a bunch of cases where we'd like to present controls in other forms - such as ClassVector headers, and in separate tool UIs, without re-implementing their display/functionality.

## Misc Other Stuff ##

ColorTransformOp and ColorSpaceTransformOp are pretty confusing - can they be consolidated into one? At what point do we let OpenColorIO do all this for us?

Use GCC visibility stuff to reduce binary sizes and linking times (http://gcc.gnu.org/wiki/Visibility). We tried this before but there were some issues - need to get Mark's input as to what they were.

Deprecate/remove classes which have been unused for a long time. The slow point distribution stuff? RadixSort? HierarchicalCache?

~~Update IE build to use a recent SCons version, and get rid of all the warnings that will come from that.~~ Addressed on nukePH branch.

Deprecate IECoreMaya::PythonCmd? We don't need it now Maya has had python for forever.

Thread safety for MessageHandler. Push/pop per thread using RAII. Default global handler settable at app startup.

Turn the unit tests into python modules which can be installed along with the others.

Add python bindings for boost::signal.

Fix OverSamplesCalculator and do todos in InterpolatedCache.

Fix IECore.ConfigLoader so we don't need to put all the import statements we need inside the functions we need them in.

Make Renderer::coordinateSystem() be scoped by Renderer::attributeBegin() and Renderer::attributeEnd(). Consider adjusting Group::render to keep it useful for coordinate systems, or adding a setTransform() method to the CoordinateSystem class.

Replace DataTraits.py where possible by making embedded typedefs on the Data classes. See GafferBindings::TypedObjectPlugBinding for an example of such a binding.

Can we remove the plugin loader for maya? Isn't there a way of having a .sog file now instead?

More use of raw pointers rather than smart pointers where appropriate - TypedDataDespatch for one.

Consider patches that still aren't committed. Faster python registry lookups for RunTimeTyped. Auto instancing for IECoreRI.

Name the renderman and arnold display drivers the same, so you don't need to know which renderer you're using to specify a socket display.