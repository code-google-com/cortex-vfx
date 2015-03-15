The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
# C++ Coding Conventions #

This simple class demonstrates the C++ coding conventions for Cortex. If this class were part of a Cortex library, it would be stored in files called "Logging.h" and "Logging.cpp" to match the name of the class that it defines. If this class defined any templated methods, the body of those methods would be stored in a separate "Logging.inl" file, which should be stored with the other include files for the library.

# Include files (.h and .inl) #

```
// License with appropriate Copyright date range and contributor credits

#ifndef IECORELIBRARYNAME_CLASSNAME_H
#define IECORELIBRARYNAME_CLASSNAME_H

// #include system headers using < >
#include <map>
#include <vector>

// #include external headers using " "
#include "boost/format.h"

// #include local headers using " "
#include "IECore/SimpleTypedData.h"

// Do not include any unnecessary headers

// Absolutely no "using namespace" statements here
 
namespace IECoreLibraryName
{

/// This is a brief (Doxygen) description of what this class does.
/// Full description of this class....
/// Classes have UpperCamelCase names and do not include under_scores. Always use tabs for indentation
/// and do not assume tab width. Use new lines between the public/protected/private sections.
class Logging
{

	public :

		typedef enum
		{
			NoLog,
			Info,
			Warning,
			Error,
			Debug,
			VerboseDebug
		} LogLevel;

		Logging();
		virtual ~Logging();

		/// Doxygen description of this public method which does something useful.
		/// Methods and arguments have lowerCamelCase names and do not include under_scores. This
		/// applies to IECore::Parameter naming conventions as well. Below are optional argument and
		/// return descriptors. Use them if you feel that the values need a more detailed explanation.
		/// @param someLevel A description of the "someLevel" argument
		/// @param msg A description of the "msg" argument
		/// @return A description of the return value
		bool logMessage( LogLevel someLevel, const std::string &msg );

		//! @name Accessors
		/// Accessors are always prefixed with set/get. get/set prefixes are never
		/// used individually. Use Doxygen grouping to mark related methods.
		//////////////////////////////////////////////////////////////
		//@{
		/// More description about the set method.
		void setLogLevel( LogLevel level );
		/// More description about the get method.
		LogLevel getLogLevel() const;
		//@}
		
		// Public members have no prefix. This non-Doxygen comment only has 2 slashes.
		LogLevel previousLogLevel;

	protected :

		// Prefix private/protected instance members with "m_". This is about the only
		// time we use under_scores.
		LogLevel m_level;

	private :
		
		// Prefix private/protected static members with "g_". This is probably the only
		// other time we use under_scores.
		static bool g_initialized;

};

} // namespace IECoreLibraryName

#endif // IECORELIBRARYNAME_CLASSNAME_H
```

# Source files (.cpp) #

```
// License with appropriate Copyright date range and contributor credits

// #include system headers using < >
#include <iostream>

// #include external headers using " "

// #include local headers using " "
#include "IECoreLibraryName/Logging.h"

// Do not include any unnecessary headers

using namespace std;
using namespace IECoreLibraryName;

Logging::Logging() : m_level( NoLog )
{
}

Logging::~Logging()
{
}

// Preference is towards a reasonable balance between horizontal and vertical space of each method. New lines in the
// middle of lengthy code statements are acceptable, but should be used sparingly. Make sure the argument names match
// those given in the include files.
bool Logging::logMessage( LogLevel someLevel, const std::string &msg )
{
	// Code blocks have the "{" brackets on seperate lines to the block definition.	
	if ( someLevel > m_level )
	{
		/// \todo This is how to mark changes that need to take place at some point
		/// in the future. Do not commit test/broken code that is commented out. Instead
		/// leave it out and add a todo.
		std::cerr << msg << endl;
	}
}

void Logging::setLogLevel( LogLevel level )
{
	previousLogLevel = m_level;
	m_level = level;
}

LogLevel Logging::getLogLevel()
{
	return m_level;
}
```