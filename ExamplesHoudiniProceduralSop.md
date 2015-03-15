The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary creating a Cortex ProceduralSop
#sidebar ExamplesTOC

# Overview #
![http://cortex-vfx.googlecode.com/svn/wiki/images/Cortexproc_icon.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/Cortexproc_icon.jpg)

This example shows you how to quickly create a **Cortex Procedural SOP** using a python function and associate it with a Cortex Procedural. You must have your procedurals installed as detailed [here](ExamplesProceduralsIntro.md).

# Usage #
```
IECoreHoudini.FnProceduralHolder.create(obj_name, procedural_type, procedural_version)
```
  * The first arg is the **string** name of the Houdini geometry OBJ that contains the ieProceduralHolder SOP.
  * The second args is the **string** name of the procedural to load.
  * The third arg is the **integer** version of the procedural to load.

# Code #
```
 import IECoreHoudini
 IECoreHoudini.FnProceduralHolder.create( "myObjectName", "myProcedural", 1 )
```