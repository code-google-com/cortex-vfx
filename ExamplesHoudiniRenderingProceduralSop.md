The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Rendering a Cortex procedural from Houdini
#sidebar ExamplesTOC

# Rendering Cortex Procedural from Houdini #
Rendering a Cortex procedural from Houdini in RenderMan requires attaching a special Geometry Shader which injects the procedural and it's parameters into the RIB.

![http://cortex-vfx.googlecode.com/svn/wiki/images/Cookbook_usingCortexRender.jpg](http://cortex-vfx.googlecode.com/svn/wiki/images/Cookbook_usingCortexRender.jpg)

The geometry shader is then assigned to the top-level object in Houdini and at render-time will evaluate the procedural and inject it's parameters into the RIB. It is really just a thin layer built on top of the `ri_dynamicload SHOP` that comes with Houdini.

For more information refer to the [Cortex IECoreHoudini docs.](http://code.google.com/p/cortex-vfx/wiki/IECoreHoudini)