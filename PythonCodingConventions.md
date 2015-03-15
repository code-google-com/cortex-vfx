The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
# Python Coding Conventions #

This simple class demonstrates the python coding conventions for Cortex. If this class were part of a Cortex python module, it would be stored in a file called "Logging.py" to match the name of the class that it defines.

# Python Files (.py) #

```
## License with appropriate Copyright date and contributor credits ##

# import system modules
import os, sys
import traceback

# import local modules
import IECore

# Do not import any unnecessary modules
# avoid "from MODULENAME import *"

## This is the Doxygen class description. Use new lines between between each method definition and its contents.
# Always use tabs for indentation and do not assume tab width.
class Logging :
	
	LogLevel = IECore.Enum.Create( "NoLog", "Info", "Warning", "Error", "Debug", "VerboseDebug" )
	
	def __init__( self ) :
	
		# private members and methods use the "__" prefix
		self.__level = Logging.LogLevel.NoLog
		
		# protected members and methods use the "_" prefix
		self._initialized = True
		
		# public members and methods do not use a prefix
		self.previousLogLevel = None
	
	## This is the method description. As with C++, methods and arguments have lowerCamelCase names
	# and do not include under_scores.
	def logMessage( self, someLevel, msg ) :
	
		if ( someLevel > self.__level ) :
			## \todo This is how to mark changes that need to take place at some point
			# in the future. Do not commit test/broken code that is commented out. Instead
			# leave it out and add a todo.
			sys.stderr.write( msg )
	
	def setLogLevel( self, level ) :
		
		self.previousLogLevel = self.__level
		self.__level = level
		
	def getLogLevel( self ) :
	
		return self.__level

```