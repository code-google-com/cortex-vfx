The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Using Cortex in a Python shell
#sidebar ExamplesTOC

# Introduction #

Cortex is not tied to an application and lots of it's functionality can be utilised from a vanilla Python shell. This can be a very useful tool for TDs.

# Code #
```
 $ python2.5
 Python 2.5.2 (r252:60911, Dec  5 2008, 10:21:08)
 [GCC 4.1.2 20071124 (Red Hat 4.1.2-42)] on linux2
 Type "help", "copyright", "credits" or "license" for more information.

 import IECore
 geo = IECore.Reader.create( "myGeometry.bgeo" ).read()
 geo.keys()

 > ['P']

 geo.numPoints

 > 100

 geo['P'].data[10:12]

 > IECore.V3fVectorData( [ IECore.V3f( 1.40451, -0.293893, 9.15294e-07 ), IECore.V3f( 1.13627, -0.293893, -0.825549 ) ] )
```