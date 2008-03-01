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

#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <setjmp.h>

#include "IECore/JPEGImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"

#include "boost/format.hpp"

extern "C"
{
#include "jpeglib.h"
}

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription <JPEGImageReader>
JPEGImageReader::m_readerDescription ("jpeg jpg");

JPEGImageReader::JPEGImageReader() :
		ImageReader( "JPEGImageReader", "Reads Joint Photographic Experts Group (JPEG) files" ),
		m_buffer(0)
{
}

JPEGImageReader::JPEGImageReader(const string & fileName) :
		ImageReader( "JPEGImageReader", "Reads Joint Photographic Experts Group (JPEG) files" ),
		m_buffer(0)
{
	m_fileNameParameter->setTypedValue( fileName );
}

JPEGImageReader::~JPEGImageReader()
{
	delete [] m_buffer;
}

bool JPEGImageReader::canRead(const string & fileName)
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if (!in.is_open())
	{
		return false;
	}

	// check the magic number of the input file
	// a jpeg should have 0xffd8ffe0 from offset 0
	unsigned int magic;
	in.seekg(0, ios_base::beg);
	in.read((char *) &magic, sizeof(unsigned int));
	return magic == 0xe0ffd8ff || magic == 0xffd8ffe0;
}

void JPEGImageReader::channelNames( vector<string> &names )
{
	names.clear();

	/// \todo Does a JPEG only ever contain these channels?
	names.push_back("R");
	names.push_back("G");
	names.push_back("B");
}

bool JPEGImageReader::isComplete()
{
	return true;
}

Imath::Box2i JPEGImageReader::dataWindow()
{
	open();

	return Box2i( V2i( 0, 0 ), V2i( m_bufferWidth - 1, m_bufferHeight - 1 ) );
}

Imath::Box2i JPEGImageReader::displayWindow()
{
	return dataWindow();
}

DataPtr JPEGImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow )
{
	open();

	int numChannels = 3;

	int channelOffset = 0;
	if ( name == "R" )
	{
		channelOffset = 0;
	}
	else if ( name == "G" )
	{
		channelOffset = 1;
	}
	else if ( name == "B" )
	{
		channelOffset = 2;
	}
	else
	{
		throw IOException( ( boost::format( "JPEGImageReader: Could not find channel \"%s\" while reading %s" ) % name % m_bufferFileName ).str() );
	}

	HalfVectorDataPtr dataContainer = new HalfVectorData();
	HalfVectorData::ValueType &data = dataContainer->writable();
	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;
	
	int dataY = 0; 
	int dataX = 0;

	for ( int y = dataWindow.min.y; y <= dataWindow.max.y; ++y, ++dataY )
	{
		HalfVectorData::ValueType::size_type dataOffset = dataY * dataWidth + dataX;
		
		for ( int x = dataWindow.min.x; x <= dataWindow.max.x; ++x, ++dataOffset )
		{
			assert( dataOffset < data.size() );

			data[dataOffset] = m_buffer[numChannels*( y * m_bufferWidth + x ) + channelOffset] / 255.0f;
		}
	}

	return dataContainer;
}

struct JPEGReaderErrorHandler : public jpeg_error_mgr
{
	jmp_buf m_jmpBuffer;
	char m_errorMessage[JMSG_LENGTH_MAX];
	
	static void errorExit ( j_common_ptr cinfo )
	{	
		assert( cinfo );
		assert( cinfo->err );
		
		JPEGReaderErrorHandler* errorHandler = static_cast< JPEGReaderErrorHandler* >( cinfo->err );
		( *cinfo->err->format_message )( cinfo, errorHandler->m_errorMessage ); 
		longjmp( errorHandler->m_jmpBuffer, 1 );
	}
	
	static void outputMessage( j_common_ptr cinfo )
	{
		assert( cinfo );
		assert( cinfo->err );
		
		char warning[JMSG_LENGTH_MAX];		
		( *cinfo->err->format_message )( cinfo, warning );		
		msg( Msg::Warning, "JPEGImageReader", warning );
	}
};

void JPEGImageReader::open()
{
	if ( fileName() != m_bufferFileName )
	{
		m_bufferFileName = fileName();

		delete [] m_buffer;
		m_buffer = 0;

		// open the file
		FILE *inFile = fopen( m_bufferFileName.c_str(), "rb" );
		if ( !inFile )
		{
			throw IOException( ( boost::format( "JPEGImageReader: Could not open file %s" ) % m_bufferFileName ).str() );
		}

		struct jpeg_decompress_struct cinfo;

		try
		{			
			JPEGReaderErrorHandler errorHandler;

			/// Setup error handler
			cinfo.err = jpeg_std_error( &errorHandler );
			
			/// Override fatal error and warning handlers
			errorHandler.error_exit = JPEGReaderErrorHandler::errorExit;
			errorHandler.output_message = JPEGReaderErrorHandler::outputMessage;			
			
			/// If we reach here then libjpeg has called our error handler, in which we've saved a copy of the
			/// error such that we might throw it as an exception.
			if ( setjmp( errorHandler.m_jmpBuffer ) )
			{				
				throw IOException( std::string( "JPEGImageReader: " ) + errorHandler.m_errorMessage );
			}

			/// Initialize decompressor to read from "inFile"
			jpeg_create_decompress( &cinfo );
			jpeg_stdio_src( &cinfo, inFile );
			jpeg_read_header( &cinfo, TRUE );

			/// Start decompression
			jpeg_start_decompress( &cinfo );

			/// Create buffer
			int rowStride = cinfo.output_width * cinfo.output_components;
			m_buffer = new unsigned char[rowStride * cinfo.output_height]();
			unsigned char *rowPointer[1];
			m_bufferWidth = cinfo.output_width;
			m_bufferHeight = cinfo.output_height;

			/// Read scanlines one at a time.
			// \todo: optimize this, probably based on image dimensions
			while (cinfo.output_scanline < cinfo.output_height)
			{
				rowPointer[0] = m_buffer + rowStride * cinfo.output_scanline;
				jpeg_read_scanlines( &cinfo, rowPointer, 1 );
			}

			/// Finish decompression
			jpeg_finish_decompress( &cinfo );
			jpeg_destroy_decompress( &cinfo );
		}
		catch ( Exception &e )
		{
			jpeg_destroy_decompress( &cinfo );
			fclose( inFile );
			
			throw;
		}
		catch ( std::exception &e )
		{
			throw IOException( ( boost::format( "JPEGImageReader : %s" ) % e.what() ).str() );
		}
		catch ( ... )
		{
			jpeg_destroy_decompress( &cinfo );
			fclose( inFile );
			
			throw IOException( "JPEGImageReader: Unexpected error" );
		}

		fclose( inFile );
	}
}
