The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Build instructions
#labels Featured


# Overview #

Cortex is built using the SCons build tool. We've intentionally not used the common autotools standard as in our experience it doesn't work well for building software in a vfx facility, where software must often be installed centrally and with specific naming conventions. We aim to make the build process work out of the box for local installs on machines with locally installed dependencies, but flexible enough to work successfully for any facility.

If you encounter problems with this process please raise them on the [cortex developers list](http://groups.google.com/group/cortexdev).

# Prerequisites #

If the SCons build tool is not available on your system it should be downloaded and installed from http://www.scons.org. You will need at least version 0.97.

You will also need the [OpenEXR](http://www.openexr.org), [boost](http://www.boost.org), and [tbb](http://www.threadingbuildingblocks.org) libraries. Internally at Image Engine we currently use OpenEXR 1.6.1, boost 1.42.0, and tbb 2.2.

Finally, you'll need at least Python version 2.4. This is preinstalled on many systems, but you may wish to download and install a version from http://www.python.org/download. Image Engine is currently using both Python 2.5 and 2.6.

Many vfx software packages use specific versions of these libraries internally. It is critical to use the same versions as the application you're imbedding Cortex into. We try to maintain a [BuildingMatrix](BuildingMatrix.md) of dependencies that have been tested in production environments. If in doubt about version compatibility, check this matrix for results that have proven successful.

# Optional Prerequisites #

If you don't already have them, you may wish to download and install the [JPEG](http://www.ijg.org/) and [TIFF](http://www.libtiff.org/) libraries, to add further image format support.

To generate the documentation you'll also need a recent version of [Doxygen](http://www.doxygen.org).

# Building #

Source code packages are available from the downloads section. Alternatively, the latest development code can be obtained from [subversion](http://code.google.com/p/cortex-vfx/source).

Assuming vanilla installs of all the dependencies in the default locations, you should simply be able to issue the following command to build the library and python module :

**scons**

If your libraries are installed in nonstandard locations you may need to add a few command line options to specify those locations. A list of available options with help is obtained with the following command :

**scons --help**

For example on my mac I install libraries to /opt/local, and therefore need something like the following :

**scons BOOST\_INCLUDE\_PATH=/opt/local/include/boost-1\_34\_1 OPENEXR\_INCLUDE\_PATH=/opt/local/include/OpenEXR LIBPATH=/opt/local/lib**

Obviously it's painful to specify these options every time you run a build, so they can be saved to file using with an extra option :

**SAVE\_OPTIONS=optionsFileName**

And loaded again with the following option :

**OPTIONS=optionsFileName**

In this latter case you can still specify further options on the command line in order to override the file contents. Furthermore, the options file can contain any executable python code, so you could edit it to import site specific modules that automatically obtain information about software installation locations etc. A somewhat complex example of such a file can be found in config/ie/options.

# Testing and documentation #

The following command runs the unit tests :

**scons test**

Note that if you used any extra command line options in the building phase you will need to add them here too - this is where the options save/load described above comes in handy.

The test results will be logged in test/$MODULENAME/results.txt.

To build the documentation the following should suffice, regardless of whether you used additional flags to build and test or not :

**scons doc**

# Installing #

Installation is simply a case of issuing the following command :

**scons install**

For peace of mind the -n command line flag can be used to display the install actions without actually executing them. This comes in handy when figuring out the command line options to specify the installation names and locations.