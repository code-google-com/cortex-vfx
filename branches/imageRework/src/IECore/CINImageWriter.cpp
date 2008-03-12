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

#include "IECore/CINImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOperators.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"

#include "IECore/private/cineon.h"

#include "boost/format.hpp"

#include <fstream>
#include <time.h>

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

const Writer::WriterDescription<CINImageWriter> CINImageWriter::m_writerDescription("cin");

CINImageWriter::CINImageWriter() :
		ImageWriter("CINImageWriter", "Serializes images to the Kodak Cineon 10-bit log image format")
{
}

CINImageWriter::CINImageWriter( ObjectPtr image, const string &fileName ) :
		ImageWriter("CINImageWriter", "Serializes images to the Kodak Cineon 10-bit log image format")
{
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

CINImageWriter::~CINImageWriter()
{
}

template<typename T>
void CINImageWriter::encodeChannel( ConstDataPtr dataContainer, const Box2i &displayWindow, const Box2i &dataWindow, int bitShift, std::vector<unsigned int> &imageBuffer )
{
	const typename T::ValueType &data = static_pointer_cast<const T>( dataContainer )->readable();
	ScaledDataConversion<typename T::ValueType::value_type, float> converter;

	int displayWidth = displayWindow.size().x + 1;
	int dataWidth = dataWindow.size().x + 1;

	int dataX = 0;
	int dataY = 0;

	for ( int y = dataWindow.min.y; y <= dataWindow.max.y; y++, dataY++ )
	{
		int dataOffset = dataY * dataWidth + dataX;
		assert( dataOffset >= 0 );

		for ( int x = dataWindow.min.x; x <= dataWindow.max.x; x++, dataOffset++ )
		{
			int pixelIdx = ( y - displayWindow.min.y ) * displayWidth + ( x - displayWindow.min.x );

			assert( pixelIdx >= 0 );
			assert( pixelIdx < (int)imageBuffer.size() );
			assert( dataOffset < (int)data.size() );

			vector<double>::iterator where = lower_bound(m_LUT.begin(), m_LUT.end(), converter( data[dataOffset] ) );
			unsigned int logValue = distance(m_LUT.begin(), where);
			imageBuffer[ pixelIdx ] |= logValue << bitShift;
		}
	}
}

void CINImageWriter::writeImage( vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow )
{
	// write the cineon in the standard 10bit log format
	std::ofstream out;
	out.open(fileName().c_str());
	if (!out.is_open())
	{
		throw IOException( "CINImageWriter: Could not open " + fileName() );
	}

	/// We'd like RGB to be at the front, in that order, because it seems that not all readers support the channel identifiers!
	vector<string> desiredChannelOrder;
	desiredChannelOrder.push_back( "R" );
	desiredChannelOrder.push_back( "G" );
	desiredChannelOrder.push_back( "B" );

	vector<string> namesCopy = names;
	vector<string> filteredNames;

	for ( vector<string>::const_iterator it = desiredChannelOrder.begin(); it != desiredChannelOrder.end(); ++it )
	{
		vector<string>::iterator res = find( namesCopy.begin(), namesCopy.end(), *it );
		if ( res != namesCopy.end() )
		{
			namesCopy.erase( res );
			filteredNames.push_back( *it );
		}
	}

	for ( vector<string>::const_iterator it = namesCopy.begin(); it != namesCopy.end(); ++it )
	{
		filteredNames.push_back( *it );
	}

	assert( names.size() == filteredNames.size() );
	
	Box2i displayWindow = image->getDisplayWindow();

	int displayWidth  = 1 + displayWindow.size().x;
	int displayHeight = 1 + displayWindow.size().y;

	// build the header
	FileInformation fi;
	fi.magic = asBigEndian<>( 0x802a5fd7 );

	fi.section_header_length = 0;
	fi.industry_header_length = 0;
	fi.variable_header_length = 0;

	strcpy(fi.version, "V4.5");

	/// \todo What is the purpose of this?
	strcpy(fi.file_name, "image-engine.cin");

	// compute the current date and time
	time_t t;
	time(&t);
	struct tm gmt;
	localtime_r(&t, &gmt);

	sprintf(fi.creation_date, "%04d-%02d-%02d", 1900 + gmt.tm_year, gmt.tm_mon, gmt.tm_mday);
	sprintf(fi.creation_time, "%02d:%02d:%02d", gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

	ImageInformation ii;
	ii.orientation = 0;
	ii.channel_count = 0;

	for (int c = 0; c < 8; ++c)
	{
		ImageInformationChannelInformation &ci = ii.channel_information[c];
		ci.byte_0 = 0;
		ci.byte_1 = 0;
		ci.bpp = 10;

		ci.pixels_per_line = 0;
		ci.lines_per_image = 0;
	}

	// write the data
	std::vector<unsigned int> imageBuffer( displayWidth*displayHeight, 0 );

	// build a LUT
	double film_gamma = 0.6;
	int ref_white_val = 685;
	int ref_black_val = 95;
	double ref_mult = 0.002 / film_gamma;
	double black_offset = pow(10.0, (ref_black_val - ref_white_val) * ref_mult);

	// build a reverse LUT (linear to logarithmic)
	m_LUT.resize(1024);
	for (int i = 0; i < 1024; ++i)
	{
		double v = i + 0.5;
		m_LUT[i] = (pow(10.0, (v - ref_white_val) * ref_mult) - black_offset) / (1.0 - black_offset);
	}

	vector<string>::const_iterator i = filteredNames.begin();
	int offset = 0;
	while (i != filteredNames.end())
	{
		if (!(*i == "R" || *i == "G" || *i == "B" || *i == "Y"))
		{
			msg( Msg::Warning, "CINImageWriter::write", format( "Channel \"%s\" was not encoded." ) % *i );
			++i;
			continue;
		}

		ImageInformationChannelInformation &ci = ii.channel_information[offset];
		ci.byte_0 = 0;

		if ( *i == "R" )
		{
			ci.byte_1 = 1;
		}
		else if ( *i == "G" )
		{
			ci.byte_1 = 2;
		}
		else if ( *i == "B" )
		{
			ci.byte_1 = 3;
		}
		else if ( *i == "Y" )
		{
			ci.byte_1 = 0;
		}

		ci.bpp = 10;

		ci.pixels_per_line = asBigEndian<>( displayWidth );
		ci.lines_per_image = asBigEndian<>( displayHeight );

		/// \todo Document these constants
		ci.min_data_value = 0.0;
		ci.min_quantity = 0.0;
		ci.max_data_value = 1023.0;
		ci.max_quantity = 2.046;

		ci.min_data_value = asBigEndian<>( ci.min_data_value );
		ci.min_quantity   = asBigEndian<>( ci.min_quantity );
		ci.max_data_value = asBigEndian<>( ci.max_data_value );
		ci.min_quantity   = asBigEndian<>( ci.max_quantity );

		int bpp = 10;
		unsigned int shift = (32 - bpp) - (offset*bpp);

		assert( image->variables.find( *i ) != image->variables.end() );
		DataPtr dataContainer = image->variables.find( *i )->second.data;
		assert( dataContainer );

		switch (dataContainer->typeId())
		{
		case FloatVectorDataTypeId:

			encodeChannel<FloatVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case HalfVectorDataTypeId:

			encodeChannel<HalfVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case DoubleVectorDataTypeId:

			encodeChannel<DoubleVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case LongVectorDataTypeId:

			encodeChannel<LongVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case CharVectorDataTypeId:

			encodeChannel<CharVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case UCharVectorDataTypeId:

			encodeChannel<UCharVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case ShortVectorDataTypeId:

			encodeChannel<ShortVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case UShortVectorDataTypeId:

			encodeChannel<UShortVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case IntVectorDataTypeId:

			encodeChannel<IntVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case UIntVectorDataTypeId:

			encodeChannel<UIntVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		default:
			throw InvalidArgumentException( (format( "CINImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(dataContainer->typeId()) % *i).str() );
		}

		ii.channel_count ++;

		++offset;

		++i;
	}

	if ( ii.channel_count < 1 || ii.channel_count > 3 )
	{
		throw IOException( "CINImageWriter: Invalid number of channels" );
	}

	ImageDataFormatInformation idfi;
	idfi.interleave = 0;               // pixel interleave
	idfi.packing = 5;                  // 32 bit left-aligned with 2 waste bits
	idfi.data_signed = 0;              // unsigned data
	idfi.sense = 0;                    // positive image sense
	idfi.eol_padding = 0;              // no end-of-line padding
	idfi.eoc_padding = 0;              // no end-of-data padding

	/// \todo Complete filling in this structure
	ImageOriginationInformation ioi;
	ioi.x_offset = 0;                  // could be dataWindow min.x
	ioi.y_offset = 0;                  // could be dataWindow min.y
	ioi.gamma = 0x7f800000;
	ioi.gamma = asBigEndian<>(ioi.gamma);


	// compute data offsets
	fi.image_data_offset = 1024;
	fi.image_data_offset = asBigEndian<>(fi.image_data_offset);

	// file size is 1024 (header) + image data size
	// image data size is 32 bits times width*height
	fi.total_file_size = 1024 + sizeof( unsigned int ) * displayWidth * displayHeight;
	fi.total_file_size = asBigEndian<>(fi.total_file_size);

	out.write(reinterpret_cast<char *>(&fi),   sizeof(fi));
	if ( out.fail() )
	{
		throw IOException( "CINImageWriter: Error writing to " + fileName() );
	}

	out.write(reinterpret_cast<char *>(&ii),   sizeof(ii));
	if ( out.fail() )
	{
		throw IOException( "CINImageWriter: Error writing to " + fileName() );
	}

	out.write(reinterpret_cast<char *>(&idfi), sizeof(idfi));
	if ( out.fail() )
	{
		throw IOException( "CINImageWriter: Error writing to " + fileName() );
	}

	out.write(reinterpret_cast<char *>(&ioi),  sizeof(ioi));
	if ( out.fail() )
	{
		throw IOException( "CINImageWriter: Error writing to " + fileName() );
	}

	// write the buffer
	for (int i = 0; i < displayWidth*displayHeight; ++i)
	{
		imageBuffer[i] = asBigEndian<>(imageBuffer[i]);
		out.write((const char *) (&imageBuffer[i]), sizeof(unsigned int));
		if ( out.fail() )
		{
			throw IOException( "CINImageWriter: Error writing to " + fileName() );
		}
	}
}
