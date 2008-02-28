//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "IECore/ImageReader.h"
#include "IECore/CompoundData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"

#include <algorithm>

using namespace std;
using namespace IECore;
using namespace boost;
using namespace Imath;

ImageReader::ImageReader( const std::string name, const std::string description ) :
		Reader( name, description, new ObjectParameter( "result", "The loaded object", new NullObject, ImagePrimitive::staticTypeId() ) )
{
	/// \todo Determine and document appropriate behaviour for occasions when requested dataWindow is outside dataWindow of the file.
	m_dataWindowParameter = new Box2iParameter(
		"dataWindow",
		"The area for which data should be loaded. The default value (an empty box) "
		"is used to specify that the full data window should be loaded. Other values may be specified "
		"to load just a section of the image.",
	);
	
	
	m_displayWindowParameter = new Box2iParameter(
		"displayWindow",
		"The displayWindow for the ImagePrimitive created during loading. The default value (an empty box) "
		"is used to specify that the displayWindow should be inferred from the file itself. On rare occasions "
		"it may be useful to specify an alternative using this parameter. Note that this parameter is completely "
		"independent of the dataWindow parameter."
	);

	m_channelNamesParameter = new StringVectorParameter( 
		"channels",
		"The names of all channels to load from the file. If the list is empty (the default value) "
		"then all channels are loaded."
	);

	parameters()->addParameter( m_dataWindowParameter );
	parameters()->addParameter( m_displayWindowParameter );
	parameters()->addParameter( m_channelNamesParameter );
}

ObjectPtr ImageReader::doOperation( ConstCompoundObjectPtr operands )
{

	// create our ImagePrimitive
	ImagePrimitivePtr image = new ImagePrimitive;

	// fetch all the user-desired channels with
	// the derived class' readChannel() implementation
	vector<string> channels;
	imageChannels(channels);

	// get the data window:
	// a user may specify a region different from the image data region,
	// and we use the empty box to indicate the full image data
	Box2i dw = dataWindow();

	// the image data window is set by the child class; it is in the position
	// to know how to determine what the defined extents of its image type are.
	//image->setDataWindow(dw);

	vector<string>::const_iterator ci = channels.begin();
	while (ci != channels.end())
	{
		readChannel(*ci, image, dw);
		ci++;
	}

	return image;
}

/// get the user-requested channel names
void ImageReader::imageChannels(vector<string> & names)
{
	vector<string> allNames;
	channelNames(allNames);

	ConstStringVectorParameterPtr p = parameters()->parameter<StringVectorParameter>("channels");
	ConstStringVectorDataPtr d = static_pointer_cast<const StringVectorData>(p->getValue());

	// give all channels when no list is provided
	if (!d->readable().size())
	{
		names = allNames;
		return;
	}

	// otherwise, copy in the requested names from the parameter set.
	// this is intersection(A, D)
	names.clear();
	for (vector<string>::const_iterator it = d->readable().begin(); it != d->readable().end(); it++)
	{
		if (find(allNames.begin(), allNames.end(), *it) != allNames.end())
		{
			names.push_back(*it);
		}
	}
}

// get the user-requested data window
Box2i ImageReader::dataWindow() const
{
	return parameters()->parameter<Box2iParameter>("dataWindow")->getTypedValue();
}

// get the user-requested display window
Box2i ImageReader::displayWindow() const
{
	return parameters()->parameter<Box2iParameter>("displayWindow")->getTypedValue();
}

Box2iParameterPtr ImageReader::dataWindowParameter()
{
	return m_dataWindowParameter;
}

ConstBox2iParameterPtr ImageReader::dataWindowParameter() const
{
	return m_dataWindowParameter;
}

Box2iParameterPtr ImageReader::displayWindowParameter()
{
	return m_displayWindowParameter;
}

ConstBox2iParameterPtr ImageReader::displayWindowParameter() const
{
	return m_displayWindowParameter;
}

StringVectorParameterPtr ImageReader::channelNamesParameter()
{
	return m_channelNamesParameter;
}

ConstStringVectorParameterPtr ImageReader::channelNamesParameter() const
{
	return m_channelNamesParameter;
}
