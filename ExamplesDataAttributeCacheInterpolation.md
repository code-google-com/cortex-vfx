The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Interpolating Attribute Caches
#sidebar ExamplesTOC

# Introduction #

This example loads a sequence of attribute cache files from disk and uses linear interpolation to extract sub-frame versions of the cached attributes. Here we extract the 'our\_attribute' attribute from the 'our\_object' object.

# Code #
```
 from IECore import *
 
 ic = InterpolatedCache( "test.####.fio", 1.0,
 			InterpolatedCache.Interpolation.Linear,
 			OversamplesCalculator( 24.0, 1, 24 ) )
 
 co = ic.read( "our_object", "our_attribute" )
 print "attribute at 1.0: ", co
 ic.setFrame(2.0)
 co = ic.read( "our_object", "our_attribute" )
 print "attribute at 2.0: ", co
 ic.setFrame(1.5)
 co = ic.read( "our_object", "our_attribute" )
 print "interpolated attribute at 1.5: ", co
```