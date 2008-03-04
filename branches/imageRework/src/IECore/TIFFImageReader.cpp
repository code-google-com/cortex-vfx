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

#include "IECore/TIFFImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ScopedTIFFExceptionTranslator.h"
#include "IECore/BoxOps.h"

#include "boost/static_assert.hpp"
#include "boost/format.hpp"

#include "tiffio.h"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription<TIFFImageReader> TIFFImageReader::m_readerDescription("tiff tif");

TIFFImageReader::TIFFImageReader()
		:	ImageReader("TIFFImageReader", "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage(0)
{
}

TIFFImageReader::TIFFImageReader( const string &fileName )
		:	ImageReader("TIFFImageReader", "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage(0)
{
	m_fileNameParameter->setTypedValue(fileName);
}

TIFFImageReader::~TIFFImageReader()
{
	ScopedTIFFExceptionTranslator errorHandler;

	if ( m_tiffImage )
	{
		TIFFClose( m_tiffImage );
		m_tiffImage = 0;
	}

}

bool TIFFImageReader::canRead( const string &fileName )
{
	ScopedTIFFExceptionTranslator errorHandler;

	// attempt to open the file
	ifstream in( fileName.c_str() );
	if ( !in.is_open() || in.fail() )
	{
		return false;
	}

	// check the magic number of the input file
	// a tiff should have 0x49492a00 / 0x002a4949 from offset 0

	// attempt to open the file
	in.seekg( 0, ios_base::beg );
	if ( in.fail() )
	{
		return false;
	}

	// check magic number
	unsigned int magic;
	in.read((char *) &magic, sizeof(unsigned int));
	if ( in.fail() )
	{
		return false;
	}

	/// \todo Why the 3 variations here? Surely only 2 are necessary?
	return magic == 0x002a4949 || magic == 0x49492a00 || magic == 0x2a004d4d;
}

void TIFFImageReader::channelNames( vector<string> &names )
{
	names.clear();

	open( true );

	/// \todo Derive channel names from header
	if ( m_samplesPerPixel >= 3 )
	{
		names.push_back("R");
		names.push_back("G");
		names.push_back("B");

		if ( m_samplesPerPixel >= 4 )
		{
			names.push_back("A");
		}
	}
}

bool TIFFImageReader::isComplete()
{
	return open( false );
}

Imath::Box2i TIFFImageReader::dataWindow()
{
	open( true );

	return m_dataWindow;
}

Imath::Box2i TIFFImageReader::displayWindow()
{
	open( true );

	return m_displayWindow;
}

template<typename T>
T TIFFImageReader::tiffField( unsigned int t )
{
	BOOST_STATIC_ASSERT( sizeof( unsigned int ) >= sizeof( ttag_t ) );
	assert( m_tiffImage );

	T tmp;
	TIFFGetField( m_tiffImage, (ttag_t)t, &tmp );
	return tmp;
}

template<typename T>
T TIFFImageReader::tiffFieldDefaulted( unsigned int t )
{
	BOOST_STATIC_ASSERT( sizeof( unsigned int ) >= sizeof( ttag_t ) );
	assert( m_tiffImage );

	T tmp;
	TIFFGetFieldDefaulted( m_tiffImage, (ttag_t)t, &tmp );
	return tmp;
}

template<typename T>
float toFloat( T t )
{
	static const double normalizer = 1.0 / Imath::limits<T>::max();

	return normalizer * t;
}

template<>
float toFloat( float t )
{
	return t;
}

template<>
float toFloat( double t )
{
	return static_cast<float>( t );
}

template<typename T>
DataPtr TIFFImageReader::readTypedChannel( const std::string &name, const Box2i &dataWindow )
{
	FloatVectorDataPtr dataContainer = new FloatVectorData();

	typename FloatVectorData::ValueType &data = dataContainer->writable();

	/// \todo
	// compute offset to image
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
	else if ( name == "A" )
	{
		channelOffset = 3;
	}
	else
	{
		throw IOException( ( boost::format( "TiffImageReader: Could not find channel \"%s\" while reading %s" ) % name % m_tiffImageFileName ).str() );
	}

	if ( channelOffset >= m_samplesPerPixel )
	{
		throw IOException( (boost::format( "TiffImageReader: Insufficient samples-per-pixel (%d) for reading channel \"%s\"") % m_samplesPerPixel % name).str() );
	}

	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;
	int bufferDataWidth = 1 + m_dataWindow.size().x;

	int dataY = 0;

	for ( int y = dataWindow.min.y; y <= dataWindow.max.y; ++y, ++dataY )
	{
		typename FloatVectorData::ValueType::size_type dataOffset = dataY * dataWidth ;

		for ( int x = dataWindow.min.x; x <= dataWindow.max.x; ++x, ++dataOffset )
		{
			assert( dataOffset < data.size() );

			const T* buf = reinterpret_cast< T* >( & m_buffer[0] );
			assert( buf );

			// \todo Currently, we only support PLANARCONFIG_CONTIG for TIFFTAG_PLANARCONFIG.
			data[dataOffset] = toFloat<T>( buf[ m_samplesPerPixel * ( y * bufferDataWidth + x ) + channelOffset ] );
		}
	}

	return dataContainer;
}

DataPtr TIFFImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow )
{
	ScopedTIFFExceptionTranslator errorHandler;

	open( true );

	if ( m_buffer.size() == 0 )
	{
		readBuffer();
	}

	if ( m_sampleFormat == SAMPLEFORMAT_IEEEFP )
	{
		return readTypedChannel<float>( name, dataWindow );
	}
	else
	{
		assert( m_sampleFormat == SAMPLEFORMAT_UINT ) ;

		switch ( m_bitsPerSample )
		{
		case 8:
			return readTypedChannel<unsigned char>( name, dataWindow );

		case 16:
			return readTypedChannel<uint16>( name, dataWindow );

		case 32:
			return readTypedChannel<uint32>( name, dataWindow );

		default:
			assert( false );
			return 0;
		}
	}
}

void TIFFImageReader::readBuffer()
{
	ScopedTIFFExceptionTranslator errorHandler;

	/// readChannel should already have opened the image by now
	assert( m_tiffImage );

	/// \todo Support tiled images!
	if ( TIFFIsTiled( m_tiffImage ) )
	{
		throw IOException( "TiffImageReader: Tiled images unsupported" );
	}

	int width = boxSize( m_dataWindow ).x + 1;
	int height = boxSize( m_dataWindow ).y + 1;

	tsize_t stripSize = TIFFStripSize(m_tiffImage);

	// \todo Currently, we only support PLANARCONFIG_CONTIG for TIFFTAG_PLANARCONFIG.
	std::vector<unsigned char>::size_type bufSize = (size_t)( (float)m_bitsPerSample / 8 * m_samplesPerPixel * width * height );
	assert( bufSize );
	m_buffer.resize( bufSize, 0 );

	// read the image
	tsize_t imageOffset = 0;
	tstrip_t numStrips = TIFFNumberOfStrips( m_tiffImage );
	for ( tstrip_t strip = 0; strip < numStrips; strip++ )
	{
		tsize_t result = TIFFReadEncodedStrip( m_tiffImage, strip, &m_buffer[0] + imageOffset, stripSize);

		if ( result == -1 )
		{
			throw IOException( (boost::format( "TiffImageReader: Read error on strip number %d") % strip).str() );
		}

		imageOffset += result;
	}
}

bool TIFFImageReader::open( bool throwOnFailure )
{
	ScopedTIFFExceptionTranslator errorHandler;

	if ( m_tiffImage )

	{
		if ( m_tiffImageFileName == fileName() )
		{
			return true;
		}
		else
		{
			TIFFClose( m_tiffImage );
			m_tiffImage = 0;
			m_buffer.clear();
		}
	}

	assert( m_tiffImage == 0 );
	assert( m_buffer.size() == 0 );

	try
	{
		m_tiffImage = TIFFOpen( fileName().c_str(), "r" );

		if (! m_tiffImage )
		{
			throw IOException( ( boost::format("TiffImageReader: Could not open %s ") % fileName() ).str() );
		}

		int width = tiffField<uint32>( TIFFTAG_IMAGEWIDTH );
		if ( width == 0 )
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_IMAGEWIDTH while reading %s") % width % fileName() ).str() );
		}

		int height = tiffField<uint32>( TIFFTAG_IMAGELENGTH );
		if ( height == 0 )
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_IMAGELENGTH while reading %s") % height % fileName() ).str() );
		}

		m_samplesPerPixel = tiffField<uint32>( TIFFTAG_SAMPLESPERPIXEL );
		if ( ! ( m_samplesPerPixel == 3 )
		   )
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_SAMPLESPERPIXEL") % m_samplesPerPixel ).str() );
		}

		m_bitsPerSample = tiffField<uint16>( TIFFTAG_BITSPERSAMPLE );
		if (! (	m_bitsPerSample == 8 ||
		                m_bitsPerSample == 16 ||
		                m_bitsPerSample == 32
		      ) )
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_BITSPERSAMPLE") % m_bitsPerSample ).str() );
		}

		m_photometricInterpretation = tiffFieldDefaulted<uint16>( TIFFTAG_PHOTOMETRIC );
		if (! ( m_photometricInterpretation == PHOTOMETRIC_MINISBLACK ||
		                m_photometricInterpretation == PHOTOMETRIC_RGB
		      ))
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_PHOTOMETRIC") % m_photometricInterpretation ).str() );
		}

		m_fillOrder = tiffFieldDefaulted<uint16>( TIFFTAG_FILLORDER );
		if (!( m_fillOrder == FILLORDER_MSB2LSB || m_fillOrder == FILLORDER_LSB2MSB) )
		{
			throw IOException( ( boost::format("TiffImageReader: Invalid value (%d) for TIFFTAG_FILLORDER") % m_fillOrder ).str() );
		}
		assert( (bool)( TIFFIsMSB2LSB( m_tiffImage) ) == (bool)( m_fillOrder == FILLORDER_MSB2LSB ));

		m_sampleFormat = tiffFieldDefaulted<uint16>( TIFFTAG_SAMPLEFORMAT );
		if (! ( m_sampleFormat == SAMPLEFORMAT_UINT ||
		                m_sampleFormat == SAMPLEFORMAT_IEEEFP
		      ))
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_SAMPLEFORMAT") % m_sampleFormat ).str() );
		}

		m_orientation = tiffFieldDefaulted<uint16>( TIFFTAG_ORIENTATION );
		if ( m_orientation != ORIENTATION_TOPLEFT )
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_ORIENTATION") % m_orientation ).str() );
		}

		m_planarConfig = tiffFieldDefaulted<uint16>( TIFFTAG_PLANARCONFIG );
		if ( m_planarConfig !=  PLANARCONFIG_CONTIG )
		{
			throw IOException( ( boost::format("TiffImageReader: Unsupported value (%d) for TIFFTAG_PLANARCONFIG") % m_orientation ).str() );
		}

		m_displayWindow = Box2i( V2i( 0, 0 ), V2i( width - 1, height - 1 ) );
		m_dataWindow = m_displayWindow;

		m_tiffImageFileName = fileName();
	}
	catch ( ... )
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

	return m_tiffImage != 0;
}

