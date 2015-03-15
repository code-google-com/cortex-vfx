The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Porting code from Cortex 4 to 5

## Introduction ##

Cortex 5 introduces a few changes which are not backwards compatible with Cortex 4. This page provides details on how to deal with them.

## Parameterised Classes ##

The Parameterised constructor in Cortex 4 used to take a "name" parameter. This was removed in Cortex 5 as it provided no functionality above and beyond the typeName() method, and encouraged lazy habits where we ended up not registering Ops and Procedurals with the type system (and therefore name and typeName() wouldn't match).

### Procedurals ###

```
# Cortex 4
###########
class MyProcedural( IECore.ParameterisedProcedural ) :

    def __init__( self ) :

        IECore.ParameterisedProcedural.__init__( self, "MyProcedural", "my description" )

# Cortex 5
##########

class MyProcedural( IECore.ParameterisedProcedural ) :

   def __init__( self ) :

        IECore.ParameterisedProcedural.__init__( self, "my description" )

# automatically implements typeName() etc and allocates a TypeId
IECore.registerRunTimeTyped( MyProcedural )

```

### Ops ###

```

# Cortex 4
##########

class MyOp( IECore.Op ) :

    def __init__( self ) :

        IECore.Op.__init__( self, "MyOp", "my description"
            IECore.IntParameter(
                "result",
                "The result parameter for the Op.",
                0
            )
        )

# Cortex 5
##########

class MyOp( IECore.Op ) :

    def __init__( self ) :

        IECore.Op.__init__( self, "my description"
            IECore.IntParameter(
                "result",
                "The result parameter for the Op.",
                0
            )
        )

# automatically implements typeName() etc and allocates a TypeId
IECore.registerRunTimeTyped( MyOp )

```

## ClassLoader ##

The ClassLoader is used by the IECoreMaya nodes which hold Ops and Procedurals, so that they can reload the Op or Procedural they need when a scene is reloaded. The filesystem layout used by the ClassLoader changed from Cortex 4 to Cortex 5, as the previous layout was peculiarly slow to traverse.

### Cortex 4 layout ###

className/classVersion/className.py

### Cortex 5 layout ###

className/className-classVersion.py

## Smart Pointers ##

Where Cortex 4 used boost::intrusive\_ptr, Cortex 5 now uses IECore::IntrusivePtr, and we now use atomic operations to keep the reference counting threadsafe. Any references to boost\_intrusive\_ptr will therefore need replacing with IECore::IntrusivePtr. Note that by using the IE\_CORE\_DECLAREPTR macro and the ObjectTypePtr, ConstObjectTypePtr, ObjectType::Ptr and ObjectType::ConstPtr typedefs you can avoid any need to reference the specific underlying pointer type.