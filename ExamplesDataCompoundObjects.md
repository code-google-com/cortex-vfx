The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Examples of working with the `CompoundObject` class in Python
#sidebar ExamplesTOC

# Overview #

The `CompoundObject` class provides string-keyed storage of Cortex objects with IO functionality. Think of it as a struct that you can assemble dynamically.

# Examples #

## Storing data on `CompoundObjects` ##
Here we store a single int and an array of floats in a `CompoundObject`.
```
from IECore import *

# create some objects
single_number = IntData( 123 )
multiple_numbers = FloatVectorData( [ 1.0, 2.0, 3.142 ] )

# create a compound object
object = CompoundObject()

# attach data to our object
object['a_single_number'] = single_number
object['some_more_numbers'] = multiple_numbers

# write our object to disk
Writer.create( object, "test.cob" ).write()

# read back into another object
object2 = Reader.create( "test.cob" ).read()
```
## Hierarchies of `CompoundObjects` ##
We can also store `CompoundObjects` within `CompoundObjects`, allowing us to create hierarchical data.
```
from IECore import *

# create an object with some data
child = CompoundObject()
child['numbers'] = FloatVectorData( [1.0, 2.0, 3.142] )
child['strings'] = StringVectorData( ["hello", "world"] )

# create another object and parent them
parent = CompoundObject()
parent['child'] = child

# write that out &amp; read it back in
Writer.create( parent, "test.cob" ).write()
copied_object = Reader.create("test.cob").read()
```

## Storing an array of `CompoundObjects` ##
We are using the `ObjectVector` class to work with arrays of `CompoundObjects`. Note that each entry in the array could have a different structure.
```
import IECore
 
# create an ObjectVector
objvec = IECore.ObjectVector()
 
# fill it with CompoundObjects
for i in range(0,5):
 
     #create an object with some data
     element = IECore.CompoundObject()
     element['id'] = IECore.IntData( i )
     element['name'] = IECore.StringData( "bob_" + str(i) )
      
     #append it to the object vector
     objvec.append(element)
 
#write that out and read it back in
IECore.Writer.create( objvec, "test.cob" ).write()
copied_objvec = IECore.Reader.create("test.cob").read()
 
#print it out
for x in range(0,len(copied_objvec)):
     print copied_objvec[x]

```
which yields
```
 IECore.CompoundObject({'id':IECore.IntData( 0 ),'name':IECore.StringData( "bob_0" )})
 IECore.CompoundObject({'id':IECore.IntData( 1 ),'name':IECore.StringData( "bob_1" )})
 IECore.CompoundObject({'id':IECore.IntData( 2 ),'name':IECore.StringData( "bob_2" )})
 IECore.CompoundObject({'id':IECore.IntData( 3 ),'name':IECore.StringData( "bob_3" )})
 IECore.CompoundObject({'id':IECore.IntData( 4 ),'name':IECore.StringData( "bob_4" )})
```