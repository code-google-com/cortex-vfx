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

#include "IECore/CINImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"

#include "IECore/private/cineon.h"

#include "boost/format.hpp"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription<CINImageReader> CINImageReader::m_readerDescription("cin");

CINImageReader::CINImageReader() :
		ImageReader( "CINImageReader", "Reads Kodak Cineon (CIN) files." )
{
}

CINImageReader::CINImageReader( const string &fileName ) :
		ImageReader( "CINImageReader", "Reads Kodak Cineon (CIN) files." )
{
	m_fileNameParameter->setTypedValue(fileName);
}

CINImageReader::~CINImageReader()
{
}

// partial validity check: assert that the file begins with the CIN magic number
bool CINImageReader::canRead( const string & fileName )
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if (!in.is_open())
	{
		return false;
	}

	// seek to start
	in.seekg(0, ios_base::beg);

	// check magic number
	unsigned int magic;
	in.read((char *) &magic, sizeof(unsigned int));
	return magic == 0xd75f2a80 || magic == 0x802a5fd7;
}

void CINImageReader::channelNames( vector<string> &names )
{
	names.clear();

	open( true );

	/// \todo read the channel names from the CIN header
	// form channel names - hardcoded for now, assume RGB (in 10bit log)
	names.push_back("R");
	names.push_back("G");
	names.push_back("B");
}


bool CINImageReader::isComplete()
{
	/// \todo
	return true;
}

Box2i CINImageReader::dataWindow()
{
	open( true );

	return Box2i( V2i( 0, 0 ), V2i( m_bufferWidth - 1, m_bufferHeight - 1 ) );
}

Box2i CINImageReader::displayWindow()
{
	open( true );

	return dataWindow();
}

/// \todo
/// we assume here CIN coding in the 'typical' configuration (output by film dumps, nuke, etc).
/// this is RGB 10bit log for film, pixel-interlaced data.  we convert this to a linear 16-bit (half)
/// format in the ImagePrimitive.
DataPtr CINImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow )
{
	if (!open())
	{
		return 0;
	}

	// kinda useless here, we have implicitly assumed log 10 bit in the surrounding code
	int bpp = 10;

	// figure out the offset into the bitstream for the given channel
	int boffset = name == "R" ? 0 : name == "G" ? 1 : 2;

	// form the mask for this channel
	unsigned int mask = 0;
	for (int pi = 0; pi < bpp; ++pi)
	{
		mask = 1 + (mask << 1);
	}
	mask <<= ((32 - bpp) - boffset * bpp);

	HalfVectorDataPtr dataContainer = new HalfVectorData();
	HalfVectorData::ValueType &data = dataContainer->writable();
	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;

	int dataY = 0;
	for ( int y = dataWindow.min.y - this->dataWindow().min.y ; y <= dataWindow.max.y - this->dataWindow().min.y ; ++y, ++dataY )
	{
		int dataX = 0;

		for ( int x = dataWindow.min.x - this->dataWindow().min.x;  x <= dataWindow.max.x - this->dataWindow().min.x ; ++x, ++dataX  )
		{
			HalfVectorData::ValueType::size_type dataOffset = dataY * dataWidth + dataX;
			assert( dataOffset < data.size() );

			unsigned int cell = reverseBytes( m_buffer[( y * m_bufferWidth + x )] );

			// assume we have 10bit log, two wasted bits aligning to 32 longword
			unsigned short cv = (unsigned short) ((mask & cell) >> (2 + (2 - boffset)*bpp));

			data[dataOffset] = m_LUT[ cv ];
		}
	}

	return dataContainer;
}

bool CINImageReader::open( bool throwOnFailure )
{
	if ( fileName() == m_bufferFileName )
	{
		return true;
	}

	try
	{
		m_bufferFileName = fileName();
		m_buffer.clear();

		ifstream in(m_bufferFileName.c_str());

		// open the file
		if (!in.is_open())
		{
			throw Exception("could not open " + fileName());
		}

		// read the header
		FileInformation fi;
		in.read(reinterpret_cast<char*>(&fi), sizeof(fi));

		// read the image information
		ImageInformation ii;
		in.read(reinterpret_cast<char*>(&ii), sizeof(ii));

		// read the data format information
		ImageDataFormatInformation idfi;
		in.read(reinterpret_cast<char*>(&idfi), sizeof(idfi));

		// grab the device information
		ImageOriginationInformation ioi;
		in.read(reinterpret_cast<char*>(&ioi), sizeof(ioi));

		// make sure we have a valid file
		// 'proper' endianness should have 802A5FD7
		if (fi.magic != 0xd75f2a80 && fi.magic != 0x802a5fd7)
		{
			throw Exception("invalid CIN magic number");
		}

		// tell the image offset
		fi.image_data_offset = reverseBytes(fi.image_data_offset);
		fi.industry_header_length = reverseBytes(fi.industry_header_length);
		fi.variable_header_length = reverseBytes(fi.variable_header_length);

		//
		// image information
		//
		for (int i = 0; i < (int)ii.channel_count; ++i)
		{

			ImageInformationChannelInformation iici = ii.channel_information[i];

			iici.pixels_per_line = reverseBytes(iici.pixels_per_line);
			iici.lines_per_image = reverseBytes(iici.lines_per_image);

			m_bufferWidth = iici.pixels_per_line;
			m_bufferHeight = iici.lines_per_image;

			if (iici.byte_0 == 1)
			{
				throw Exception("vendor specific CIN files are not handled");
			}

			// check colour
			if (!(iici.byte_1 == 1 || iici.byte_1 == 2 || iici.byte_1 == 3))
			{
				throw Exception("CIN with non-RGB channel data not handled");
			}
		}

		//
		// device information
		//

		// seek to the image data offset
		in.seekg(fi.image_data_offset, ios_base::beg);

		// build a LUT for 10bit log -> 16bit linear conversion

		// get reference white
		//double film_gamma = 1.7;
		double film_gamma = 0.6;
		int ref_black_val = 95;
		int ref_white_val = 685;
		double quantization_step = 0.002;
		double ref_mult = quantization_step / film_gamma;

		// compute black offset
		double black_offset = pow(10.0, (ref_black_val - ref_white_val) * ref_mult);

		// standard lut
		for (int i = 0; i < 1024; ++i)
		{
			float cvf = (pow(10.0, (i - ref_white_val) * ref_mult) - black_offset) / (1.0 - black_offset);
			m_LUT[i] = cvf;
		}

		//
		// read in the buffer
		//

		// determine the size
		int buffersize = 3 * m_bufferWidth * m_bufferHeight;

		// assume data is interlaced, just read the whole thing,
		// then stripe off the channel
		m_buffer.resize( buffersize );

		// read the data into the buffer
		in.read((char *)(&m_buffer[0]), sizeof(unsigned int) * buffersize);
	}
	catch (...)
	{
		if ( throwOnFailure )
		{
			throw;
		}
		else
		{
			return false;
		}
	}

	return true;
}

