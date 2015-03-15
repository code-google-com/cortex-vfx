The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Creating an attribute cache
#sidebar ExamplesTOC

# Introduction #

This example creates 2 frame sequence of attribute cache files on disk. Each one storing a vector of floats in the notional 'our\_object' object under the 'our\_attribute' attribute. The concept of objects and attributes are loose when dealing with attribute caches and can be used to assign some context to arbitrary data in a cache. For example, object 'head' and attribute 'P' could be used to describe the point positions of vertices on the head object.

# Code #
```
 from IECore import * 
 f1 = FloatVectorData( [1.0, 2.0, 3.0] )
 f2 = FloatVectorData( [2.0, 4.0, 6.0] )
 
 ac = AttributeCache( "test.0001.fio", IndexedIOOpenMode.Write )
 ac.write( "our_object", "our_attribute", f1 )
 
 ac = AttributeCache( "test.0002.fio", IndexedIOOpenMode.Write )
 ac.write( "our_object", "our_attribute", f2 )
```