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

#include <setjmp.h>
#include <stdio.h>

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
	g_classData.erase( this );
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


struct JPEGWriterErrorHandler : public jpeg_error_mgr
{
	jmp_buf m_jmpBuffer;
	char m_errorMessage[JMSG_LENGTH_MAX];

	static void errorExit ( j_common_ptr cinfo )
	{
		assert( cinfo );
		assert( cinfo->err );

		JPEGWriterErrorHandler* errorHandler = static_cast< JPEGWriterErrorHandler* >( cinfo->err );
		( *cinfo->err->format_message )( cinfo, errorHandler->m_errorMessage );
		longjmp( errorHandler->m_jmpBuffer, 1 );
	}

	static void outputMessage( j_common_ptr cinfo )
	{
		assert( cinfo );
		assert( cinfo->err );

		char warning[JMSG_LENGTH_MAX];
		( *cinfo->err->format_message )( cinfo, warning );
		msg( Msg::Warning, "JPEGImageWriter", warning );
	}
};

void JPEGImageWriter::writeImage( vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow )
{
	FILE *outFile = 0;
	if ((outFile = fopen(fileName().c_str(), "wb")) == NULL)
	{
		throw IOException("Could not open '" + fileName() + "' for writing.");
	}
	assert( outFile );

	int width  = 1 + dataWindow.max.x - dataWindow.min.x;
	int height = 1 + dataWindow.max.y - dataWindow.min.y;
	int numChannels = 3;

	// compression info
	struct jpeg_compress_struct cinfo;

	try
	{
		JPEGWriterErrorHandler errorHandler;

		/// Setup error handler
		cinfo.err = jpeg_std_error( &errorHandler );

		/// Override fatal error and warning handlers
		errorHandler.error_exit = JPEGWriterErrorHandler::errorExit;
		errorHandler.output_message = JPEGWriterErrorHandler::outputMessage;

		/// If we reach here then libjpeg has called our error handler, in which we've saved a copy of the
		/// error such that we might throw it as an exception.
		if ( setjmp( errorHandler.m_jmpBuffer ) )
		{
			throw IOException( std::string( "JPEGImageWriter: " ) + errorHandler.m_errorMessage );
		}

		jpeg_create_compress( &cinfo );

		jpeg_stdio_dest(&cinfo, outFile);

		cinfo.image_width = width;
		cinfo.image_height = height;
		cinfo.input_components = numChannels;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults( &cinfo );

		int quality = qualityParameter()->getNumericValue();

		// force baseline-JPEG (8bit) values with TRUE
		jpeg_set_quality(&cinfo, quality, TRUE);

		int rowStride = width * numChannels;

		// build the buffer
		std::vector<unsigned char> imageBuffer( width*height*numChannels, 0 );

		// add the channels into the header with the appropriate types
		// channel data is RGB interlaced
		for ( vector<string>::const_iterator it = names.begin(); it != names.end(); it++ )
		{
			const string &name = *it;

			if (!(name == "R" || name == "G" || name == "B"))
			{
				msg( Msg::Warning, "JPEGImageWriter", format( "Channel \"%s\" was not encoded." ) % name );
				continue;
			}

			int channelOffset = 0;
			if ( name == "R" )
			{
				channelOffset = 0;
			}
			else if ( name == "G" )
			{
				channelOffset = 1;
			}
			else
			{
				assert( name == "B" );
				channelOffset = 2;
			}

			// get the image channel
			assert( image->variables.find( name ) != image->variables.end() );			
			DataPtr dataContainer = image->variables.find( name )->second.data;
			assert( dataContainer );

			switch ( dataContainer->typeId() )
			{

			case FloatVectorDataTypeId:
			{
				const FloatVectorData::ValueType &data = static_pointer_cast<FloatVectorData>( dataContainer )->readable();

				// convert to 8-bit integer
				for (int i = 0; i < width*height; ++i)
				{
					imageBuffer[numChannels*i + channelOffset] = (unsigned char) (max(0.0, min(255.0, 255.0 * data[i] + 0.5)));
				}
			}
			break;

			case UIntVectorDataTypeId:
			{
				const UIntVectorData::ValueType &data = static_pointer_cast<UIntVectorData>( dataContainer )->readable();

				// convert to 8-bit integer
				for (int i = 0; i < width*height; ++i)
				{
					// \todo: round here
					BOOST_STATIC_ASSERT( sizeof( unsigned int ) == 4 );
					imageBuffer[numChannels*i + channelOffset] = (unsigned char) (data[i] >> 24);
				}
			}
			break;

			case HalfVectorDataTypeId:
			{
				const HalfVectorData::ValueType &data = static_pointer_cast<HalfVectorData>( dataContainer )->readable();

				// convert to 8-bit linear integer
				for (int i = 0; i < width*height; ++i)
				{
					imageBuffer[numChannels*i + channelOffset] = (unsigned char) (max(0.0, min(255.0, 255.0 * data[i] + 0.5)));
				}
			}
			break;

			/// \todo Deal with other channel types, preferably using templates!

			default:
				throw InvalidArgumentException( (format( "JPEGImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(dataContainer->typeId()) % name).str() );
			}
		}

		// start the compressor
		jpeg_start_compress(&cinfo, TRUE);

		// pass one scanline at a time
		unsigned char *rowPointer[1];
		while (cinfo.next_scanline < cinfo.image_height)
		{
			rowPointer[0] = &imageBuffer[cinfo.next_scanline * rowStride];
			jpeg_write_scanlines(&cinfo, rowPointer, 1);
		}

		jpeg_finish_compress( &cinfo );
		jpeg_destroy_compress( &cinfo );
		fclose( outFile );

	}
	catch ( Exception &e )
	{
		jpeg_destroy_compress( &cinfo );
		fclose( outFile );

		throw;
	}
	catch ( std::exception &e )
	{
		throw IOException( ( boost::format( "JPEGImageReader : %s" ) % e.what() ).str() );
	}
	catch ( ... )
	{
		jpeg_destroy_compress( &cinfo );
		fclose( outFile );

		throw IOException( "JPEGImageReader: Unexpected error" );
	}
}
