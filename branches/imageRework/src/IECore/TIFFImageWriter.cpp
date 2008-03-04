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

#include "boost/static_assert.hpp"
#include "boost/format.hpp"

#include "IECore/TIFFImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/Parameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/ScopedTIFFExceptionTranslator.h"

#include "IECore/BoxOperators.h"

#include "tiffio.h"

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

const Writer::WriterDescription<TIFFImageWriter> TIFFImageWriter::m_writerDescription("tiff tif");

TIFFImageWriter::TIFFImageWriter() 
	: 	ImageWriter("TIFFImageWriter", "Serializes images to the Tagged Image File Format (TIFF) format")
{
	constructParameters();
}

TIFFImageWriter::TIFFImageWriter(ObjectPtr image, const string & fileName) 
	: 	ImageWriter("TIFFImageWriter", "Serializes images to the Tagged Image File Format (TIFF) format")
{
	constructParameters();
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

void TIFFImageWriter::constructParameters( )
{
	// bitdepth parameter
	IntParameter::PresetsMap bitDepthPresets;
	bitDepthPresets["8"] = 8;
	bitDepthPresets["16"] = 16;	
	bitDepthPresets["32"] = 32;	
	
	m_bitdepthParameter = new IntParameter(
		"bitdepth", 
		"Output TIFF bit depth",
		16,
		8,
		32,
		bitDepthPresets,
		true);
		
	parameters()->addParameter(m_bitdepthParameter);

	// compression parameter
	IntParameter::PresetsMap compressionPresets;
	compressionPresets["none"]    = COMPRESSION_NONE;
	compressionPresets["lzw"]     = COMPRESSION_LZW;
	compressionPresets["jpeg"]    = COMPRESSION_JPEG;
	compressionPresets["deflate"] = COMPRESSION_DEFLATE;

	/// Verify min
	BOOST_STATIC_ASSERT( COMPRESSION_NONE < COMPRESSION_LZW );	
	BOOST_STATIC_ASSERT( COMPRESSION_NONE < COMPRESSION_JPEG );		
	BOOST_STATIC_ASSERT( COMPRESSION_NONE < COMPRESSION_DEFLATE );	

	/// Verify max
	BOOST_STATIC_ASSERT( COMPRESSION_DEFLATE > COMPRESSION_NONE );	
	BOOST_STATIC_ASSERT( COMPRESSION_DEFLATE > COMPRESSION_LZW );		
	BOOST_STATIC_ASSERT( COMPRESSION_DEFLATE > COMPRESSION_JPEG );


	m_compressionParameter = new IntParameter("compression", "image data compression method",
		compressionPresets["lzw"], 
		COMPRESSION_NONE,
		COMPRESSION_DEFLATE,
		compressionPresets, 
		true);
		
	parameters()->addParameter(m_compressionParameter);
}

TIFFImageWriter::~TIFFImageWriter()
{
}

void TIFFImageWriter::stripEncode(tiff * tiffImage, char * imageBuffer, int imageBufferSize, int strips)
{	
	assert( tiffImage );
	assert( imageBuffer );	
	assert( imageBufferSize >= 0 );
	assert( strips >= 0 );
	
	// write the information to the file
	int offset = 0;

	for(int strip = 0; strip < strips; ++strip)
	{
		int tss = TIFFStripSize(tiffImage);
		assert( tss >= 0 );
		
		int remaining = imageBufferSize - offset;
		assert( remaining >= 0 );
		
		int lc = TIFFWriteEncodedStrip(tiffImage, strip, imageBuffer + offset, tss < remaining ? tss : remaining);
		assert( lc >= 0 );
		
		offset += lc;
	}
}


void TIFFImageWriter::writeImage(vector<string> & names, ConstImagePrimitivePtr image, const Box2i & dw)
{	
	ScopedTIFFExceptionTranslator errorHandler( );

	// create the tiff file
	TIFF *tiffImage;
	if((tiffImage = TIFFOpen(fileName().c_str(), "w")) == NULL)
	{
		throw IOException("Could not open '" + fileName() + "' for writing.");
	}

	// compute the writebox	
	int width  = 1 + dw.max.x - dw.min.x;
	int height = 1 + dw.max.y - dw.min.y;

	// compute the number of channels
	int spp = names.size();

	/// \todo different compression methods have a bearing on other attributes, eg. the strip size
	/// handle these issues a bit better and perhaps more explicitly here.  also, we should probably
	/// warn the user in cases where parameter settings are not permitted (eg 16 bit jpeg)
	int compression = parameters()->parameter<IntParameter>("compression")->getNumericValue();
	TIFFSetField(tiffImage, TIFFTAG_COMPRESSION, compression);

	// read the bitdepth parameter, default to 16 bits
	int bits = compression == COMPRESSION_JPEG ? 8 : parameters()->parameter<IntParameter>("bitdepth")->getNumericValue();	
	int bps = bits == 8 || bits == 16 || bits == 32 ? bits : 16;
	//bits = bits;
	
	/// \todo have the code below handle the various bitdepths
	// hardcode to 16 bits bitdepth
	//int bps = 16;
	int bpc = bps / 8;

	// number of strips to write.  TIFF's JPEG compression requires rps to be a multiple of 8
	int rowsPerStrip = 8;
	
	// now properly compute # of strips
	int strips = height / rowsPerStrip + (height % rowsPerStrip > 0 ? 1 : 0);
	
	// set the basic values
	TIFFSetField(tiffImage, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tiffImage, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(tiffImage, TIFFTAG_BITSPERSAMPLE, bps);
	TIFFSetField(tiffImage, TIFFTAG_SAMPLESPERPIXEL, spp);
	TIFFSetField(tiffImage, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);
	
	TIFFSetField(tiffImage, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tiffImage, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(tiffImage, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	/// \todo output float tiffs if desired
	TIFFSetField(tiffImage, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);

	//PLANARCONFIG_CONTIG    (pixel interlaced)
	//PLANARCONFIG_SEPARATE  (channel-interleave)
	
	TIFFSetField(tiffImage, TIFFTAG_XRESOLUTION, 1);
	TIFFSetField(tiffImage, TIFFTAG_YRESOLUTION, 1);
	TIFFSetField(tiffImage, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);

	//
	// encode
	//
	switch(bps) {

	case 8:
	{
		unsigned char * v = encodeChannels<unsigned char>(image, names, dw);
		assert( v );
		stripEncode(tiffImage, (char *) v, bpc * width * height * spp, strips);
		delete [] v;
	}
	break;

	case 16:
	{
		/// \todo An unsigned short needn't be 16 bits long. Use typedefs from <stdint.h> instead.
		BOOST_STATIC_ASSERT( sizeof( unsigned short ) == 2 );
		unsigned short * v = encodeChannels<unsigned short>(image, names, dw);
		assert( v );
		stripEncode(tiffImage, (char *) v, bpc * width * height * spp, strips);
		delete [] v;
	}
	break;
	
	case 32:
	{
		/// \todo An unsigned int needn't be 32 bits long. Use typedefs from <stdint.h> instead.
		BOOST_STATIC_ASSERT( sizeof( unsigned int ) == 4 );
		unsigned int * v = encodeChannels<unsigned int>(image, names, dw);
		assert( v );		
		stripEncode(tiffImage, (char *) v, bpc * width * height * spp, strips);
		delete [] v;
	}
	break;

	}
	
	// close the file
	TIFFClose(tiffImage);
	tiffImage = 0;	
}
