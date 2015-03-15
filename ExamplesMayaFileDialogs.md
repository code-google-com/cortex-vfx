The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary Demonstrates a simple addition to the 'right click' parameter menu to allow export of values to disk.
#sidebar ExamplesTOC

# Introduction #

In Cortex, the **IECoreMaya.FileDialog** class provides an alternative to **maya.cmds.fileDialog**. More details here:

[IECoreMaya.FileDialog](http://opensource.image-engine.com/doc/cortex-5.22.0/classIECoreMaya_1_1FileDialog_1_1FileDialog.html#_details)

[IECoreMaya.FileBrowser](http://opensource.image-engine.com/doc/cortex-5.22.0/classIECoreMaya_1_1FileBrowser_1_1FileBrowser.html#_details)

Here's a really quick example of how to pop up a dialog box from the right-click Parameter menu,
that asks the user where to save the value for the parameter, enforcing that they specify the extension '.cob'.

# Details #

You may wish to put this code in one of your in-house modules, or somewhere equally suitable.

```
## Generally, we don't actually want to export these functions
__all__ = []

## Define a function that the menu item will call, this does the actual work
## of saving the value
def __saveParameterValue( parameter, node ) :

	# For more on why were making a class for this, see the bottom of this page
	class Exporter :
	
		def __init__( self, node, parameter ) :
		
			self.__node = node
			self.__parameter = parameter
		
		def doExport( self, paths ) :		
			
			# Paths will be empty if the user has cancelled
			if not paths :
				return
			
			# Make sure that the value in the Parameterised object is up to date
			fn = IECoreMaya.FnParameterisedHolder( self.__node )
			fn.setParameterisedValue( self.__parameter )
			data = self.__parameter.getValue()
			
			IECore.Writer.create( data, paths[0] ).write()
	
	# Make an instance of the file extension filter, so we can
	# make the dialog more picky
	tools = IECoreMaya.FileBrowser.FileExtensionFilter( [ "cob" ] )
	
	exporter = Exporter( node, parameter )

	IECoreMaya.FileDialog(
		title = "Enter a file name (.cob) to save to",
		buttonTitle = "Export",
		saveMode = True,
		key = "parameterValueExportTool",
		filter = tools.filter,
		validate = tools.validate,
		# Register the method of our Exporter instance as the callback
		callback = exporter.doExport,		
	)

## We need to define a callback that will be called each time the parameter
## right-click menu is displayed. Its purpose is to determine whether or
## not the parameter in question should show our menu item.
## One way to achieve this on a per-parameter basis is to use an entry
## in the parameters userData(), which is what we do here.
def __menuCallback( definition, parameter, node ) :
	
	active = False
	with IECore.IgnoredExceptions( KeyError ) :
		active = parameter.userData()["maya"]["useValueExportTool"].value		
		
	if not active :
		return
	
	definition.append( "useValueExportTool", { "divider" : True } )
	definition.append( "Export Value to disk...", { "command" : IECore.curry( __saveParameterValue, parameter, node ) } )
	
## This should be called once to register our callback with the menu
IECoreMaya.ParameterUI.registerPopupMenuCallback( __menuCallback )
```

Then, for each parameter you wish this to be used on, wherever this parameter is defined, you can do something like this

```
IECore.ObjectParameter(
	name = "geometry",
	description = "The geometry to render.",
	defaultValue = IECore.Group(),
	types = [ IECore.VisibleRenderable.staticTypeId(), IECore.ObjectVector.staticTypeId() ],
	userData = {
		"maya" : {
			"useValueExportTool" : IECore.BoolData( True ),
		},
	},
)
```


# Other tricks #

The FileDialog call takes all the keyword arguments of the FileBrowser class. One of these is the **options** kw.
This allows you to provide a callable with the following signature

> `f( <FileBrowser instance>, uiParent )`


Making Exporter a class instead of just using a curried function becomes more relevent then.
You could create something like this should you wish, which adds a text field to the dialog, and stores the text in the header of the resulting file.:

```
import maya.cmds

...

def __saveParameterValue( parameter, node ) :

	Class Exporter :
 
		...

		def drawOptions( self, fileBrowserInstance, uiParent ) :

			self.__description = maya.cmds.textField( parent=uiParent )

		def doExport( self, paths ) :

			...

			w = IECore.Writer.create( data, paths[0] )

			description = maya.cmds.textField( self.__description, query=True, text=True )
			w["header"].getValue()["description"] = IECore.StringData( description )

			w.write()
			
	...
	
	IECoreMaya.FileDialog(
		
		...
		
		options = exporter.drawOptions,		
	)	
```