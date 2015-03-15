The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary List of changes in Cortex 5

## New features ##

  * New ClassParameter and ClassVectorParameter classes, along with handlers and uis for them in IECoreMaya. These allow Ops and Procedurals (and anything else deriving from Parameterised) to grow new behaviours dynamically by embedding other Ops and Procedurals.

  * New C++ and  GLSL implementations of the Marschner hair shading model, to match the existing RSL implementation.

  * New SXRenderer class provides basic support for using 3delight's Sx API from cortex

  * New algorithms for processing spherical harmonics

  * New classes :
    * IgnoredExceptions context manager for python
    * ScopedGILLock and ScopedGILRelease to aid in correct juggling of the Python GIL
    * CurveTangentsOp
    * IDXReader reads point clouds from geo survey format
    * ImageConvolveOp performs fast diffuse convolution of environment maps
    * WorldBlock and TransformBlock for correctly scoping worldBegin/End and transformBegin/End pairs
    * TestCase and TestProgram classes in IECoreMaya to make unit tests for other modules much simpler

  * PresetManager class to allow users to save and load arbitrary sets of presets for Parameterised classes

## Improvements ##


  * Support for specifying coshaders through IECoreRI::Renderer
  * ParameterParser - parameter can now opt out of serialisation and parsing
  * Friendlier member accessors for CompoundObject and CompoundData
  * Many improvements to IECoreGL, which is now moved out of contrib and considered part of the main package
  * Many thread safety (50% less finger crossing!)
  * IECoreNuke now works with Nuke 5.2 and 6.0
  * Improved handling of colorspaces for Image IO classes
  * Python binding and optimisations for InverseDistanceWeightedInterpolation
  * Optimisations for ClassLoader