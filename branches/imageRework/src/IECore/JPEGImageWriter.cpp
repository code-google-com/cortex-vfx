//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/JPEGImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ClassData.h"
#include "IECore/CompoundParameter.h"

#include "boost/format.hpp"

#include <fstream>

extern "C"
{
#include "jpeglib.h"
}
#include <stdio.h>

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

const Writer::WriterDescription<JPEGImageWriter> JPEGImageWriter::m_writerDescription("jpeg jpg");

struct JPEGImageWriter::ExtraData
{
	IntParameterPtr m_qualityParameter;
};

typedef ClassData< JPEGImageWriter, JPEGImageWriter::ExtraData*, Deleter<JPEGImageWriter::ExtraData*> > JPEGImageWriterClassData;
static JPEGImageWriterClassData g_classData;


JPEGImageWriter::JPEGImageWriter() :
		ImageWriter("JPEGImageWriter", "Serializes images to the Joint Photographic Experts Group (JPEG) format")
{
	g_classData.create( this, new ExtraData() );
	constructParameters();
}

JPEGImageWriter::JPEGImageWriter(ObjectPtr image, const string &fileName) :
		ImageWriter("JPEGImageWriter", "Serializes images to the Joint Photographic Experts Group (JPEG) format")
{
	g_classData.create( this, new ExtraData() );
	constructParameters();
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

void JPEGImageWriter::constructParameters()
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );

	extraData->m_qualityParameter = new IntParameter(
	        "quality",
	        "The quality at which to compress the JPEG. 100 yields the largest file size, but best quality image.",
	        100,
	        0,
	        100
	);

	parameters()->addParameter( extraData->m_qualityParameter );
}

JPEGImageWriter::~JPEGImageWriter()
{
}

IntParameterPtr JPEGImageWriter::qualityParameter()
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );

	return extraData->m_qualityParameter;
}

ConstIntParameterPtr JPEGImageWriter::qualityParameter() const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );

	return extraData->m_qualityParameter;
}

void JPEGImageWriter::writeImage(vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dw)
{

	// open the output file
	FILE *outfile;
	if ((outfile = fopen(fileName().c_str(), "wb")) == NULL)
	{
		throw IOException("Could not open '" + fileName() + "' for writing.");
	}

	/// \todo libjpeg error handling

	// assume an 8-bit RGB image
	int width  = 1 + dw.max.x - dw.min.x;
	int height = 1 + dw.max.y - dw.min.y;
	int spp = 3;

	// build the buffer
	std::vector<unsigned char> image_buffer( width*height*spp, 0 );
	unsigned char *row_pointer[1];
	int row_stride;

	// compression info
	struct jpeg_compress_struct cinfo;

	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = spp;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);

	int quality = qualityParameter()->getNumericValue();

	// force baseline-JPEG (8bit) values with TRUE
	jpeg_set_quality(&cinfo, quality, TRUE);

	row_stride = width * spp;

	// add the channels into the header with the appropriate types
	// channel data is RGB interlaced
	vector<string>::const_iterator i = names.begin();
	while (i != names.end())
	{

		if (!(*i == "R" || *i == "G" || *i == "B"))
		{
			msg( Msg::Warning, "JPEGImageWriter", format( "Channel \"%s\" was not encoded." ) % *i );
			++i;
			continue;
			//throw Exception("invalid channel for JPEG writer, channel name is: " + *i);
		}

		int offset = *i == "R" ? 0 : *i == "G" ? 1 : 2;

		// get the image channel
		DataPtr channelp = image->variables.find( *i )->second.data;
		assert( channelp );

		switch (channelp->typeId())
		{

		case FloatVectorDataTypeId:
		{
			const vector<float> &channel = static_pointer_cast<FloatVectorData>(channelp)->readable();

			// convert to 8-bit integer
			for (int i = 0; i < width*height; ++i)
			{
				image_buffer[spp*i + offset] = (unsigned char) (max(0.0, min(255.0, 255.0 * channel[i] + 0.5)));
			}
		}
		break;

		case UIntVectorDataTypeId:
		{
			const vector<unsigned int> &channel = static_pointer_cast<UIntVectorData>(channelp)->readable();

			// convert to 8-bit integer
			for (int i = 0; i < width*height; ++i)
			{
				// \todo: round here
				BOOST_STATIC_ASSERT( sizeof( unsigned int ) == 4 );
				image_buffer[spp*i + offset] = (unsigned char) (channel[i] >> 24);
			}
		}
		break;

		case HalfVectorDataTypeId:
		{
			const vector<half> &channel = static_pointer_cast<HalfVectorData>(channelp)->readable();

			// convert to 8-bit linear integer
			for (int i = 0; i < width*height; ++i)
			{
				image_buffer[spp*i + offset] = (unsigned char) (max(0.0, min(255.0, 255.0 * channel[i] + 0.5)));
			}
		}
		break;

		/// \todo Deal with other channel types, preferably using templates!

		default:
			throw InvalidArgumentException( (format( "JPEGImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(channelp->typeId()) % *i).str() );
		}

		++i;
	}

	// start the compressor
	jpeg_start_compress(&cinfo, TRUE);

	// pass one scanline at a time
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outfile);
	outfile = 0;
}
