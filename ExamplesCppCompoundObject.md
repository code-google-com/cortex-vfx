The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Working with Compound Objects in CPP
#sidebar ExamplesTOC
# Overview #

This example creates a CompoundObject, assigns some data to it and then saves the object to disk as an attribute cache.

Provided the data contained within a CompoundObject is interpolatable itself, then using an InterpolatedCache object to read the object back in will give the expected, interpolated, CompoundObject.

# Code #
```
 #include <OpenEXR/ImathVec.h>;
 
 #include <IECore/CompoundObject.h>;
 #include <IECore/VectorTypedData.h>;
 #include <IECore/AttributeCache.h>;
 using namespace IECore;
 
 int main( int argc, char *argv[] )
 {
 	// create our container
 	CompoundObjectPtr container( new CompoundObject() );
 
 	// create an array of ints
 	IntVectorDataPtr int_data( new IntVectorData() );
 	int_data->writable().push_back( 1 );
 	int_data->writable().push_back( 2 );
 	int_data->writable().push_back( 3 );
 
 	// create an array of V3fs
 	V3fVectorDataPtr vec_data( new V3fVectorData() );
 	vec_data->writable().push_back( Imath::V3f(1,1,1) );
 	vec_data->writable().push_back( Imath::V3f(2,2,2) );
 	vec_data->writable().push_back( Imath::V3f(3,3,3) );
 
 	// add the data to our container
 	container->members()["our_int_data"] = int_data;
 	container->members()["our_vec_data"] = vec_data;
 
 	// create a new attribute cache
 	AttributeCachePtr cache( new AttributeCache( "test.0001.fio", IndexedIO::Write ) );
 
 	// write our container to disk
 	cache->write( "our_object", "our_attribute", container );
 
 	// we discard the cache object to complete writing.
 	// not actually necessary as it's about to go out of scope
 	// anyhow, but left to make this example clear.
 	cache.reset();
 
 	return 0;
 }
```

# Compile #
```
  g++ -o CompoundObjectExample \
 	 -I$CORTEX_ROOT/include \
 	 -I$BOOST_ROOT/include/boost-1_38 \
 	 -I$ILMBASE_ROOT/include \
 	 -I$OPENEXR_ROOT/include/OpenEXR \
 	 -L$CORTEX_ROOT/lib \
 	 CompoundObjectExample.cpp -lIECore
```