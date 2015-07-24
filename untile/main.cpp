#include <cstring>
#include <cstdio>
#include <cstdint>

typedef int GLint;
typedef unsigned char GLubyte;
struct radeon_renderbuffer {
	struct {
		int Height;
	} base;
	int pitch;
	int cpp;	// byte per pixel
	int group_bytes;
	int num_channels;	// same as pipes in r800 and above
	int num_banks;
	int r7xx_bank_op;
};

// Following code is derived from the legacy Mesa r600 driver (radeon_span.c).

/**************************************************************************

Copyright (C) The Weather Channel, Inc.  2002.  All Rights Reserved.
Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

The Weather Channel (TM) funded Tungsten Graphics to develop the
initial release of the Radeon 8500 driver under the XFree86 license.
This notice must be preserved.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *   Keith Whitwell <keith@tungstengraphics.com>
 *
 */

static inline GLint r600_coord_within_microtile(GLint x, GLint y, GLint element_bytes)
{
        GLint pixel_number = 0;
	    switch (element_bytes) {
	    case 1:
		    pixel_number |= ((x >> 0) & 1) << 0; // pn[0] = x[0]
		    pixel_number |= ((x >> 1) & 1) << 1; // pn[1] = x[1]
		    pixel_number |= ((x >> 2) & 1) << 2; // pn[2] = x[2]
		    pixel_number |= ((y >> 1) & 1) << 3; // pn[3] = y[1]
		    pixel_number |= ((y >> 0) & 1) << 4; // pn[4] = y[0]
		    pixel_number |= ((y >> 2) & 1) << 5; // pn[5] = y[2]
		    break;
	    case 2:
		    pixel_number |= ((x >> 0) & 1) << 0; // pn[0] = x[0]
		    pixel_number |= ((x >> 1) & 1) << 1; // pn[1] = x[1]
		    pixel_number |= ((x >> 2) & 1) << 2; // pn[2] = x[2]
		    pixel_number |= ((y >> 0) & 1) << 3; // pn[3] = y[0]
		    pixel_number |= ((y >> 1) & 1) << 4; // pn[4] = y[1]
		    pixel_number |= ((y >> 2) & 1) << 5; // pn[5] = y[2]
		    break;
	    case 4:
		    pixel_number |= ((x >> 0) & 1) << 0; // pn[0] = x[0]
		    pixel_number |= ((x >> 1) & 1) << 1; // pn[1] = x[1]
		    pixel_number |= ((y >> 0) & 1) << 2; // pn[2] = y[0]
		    pixel_number |= ((x >> 2) & 1) << 3; // pn[3] = x[2]
		    pixel_number |= ((y >> 1) & 1) << 4; // pn[4] = y[1]
		    pixel_number |= ((y >> 2) & 1) << 5; // pn[5] = y[2]
		    break;
	    case 8:
		    pixel_number |= ((x >> 0) & 1) << 0; // pn[0] = x[0]
		    pixel_number |= ((y >> 0) & 1) << 1; // pn[1] = y[0]
		    pixel_number |= ((x >> 1) & 1) << 2; // pn[2] = x[1]
		    pixel_number |= ((x >> 2) & 1) << 3; // pn[3] = x[2]
		    pixel_number |= ((y >> 1) & 1) << 4; // pn[4] = y[1]
		    pixel_number |= ((y >> 2) & 1) << 5; // pn[5] = y[2]
		    break;
	    case 16:
		    pixel_number |= ((y >> 0) & 1) << 0; // pn[0] = y[0]
		    pixel_number |= ((x >> 0) & 1) << 1; // pn[1] = x[0]
		    pixel_number |= ((x >> 1) & 1) << 2; // pn[2] = x[1]
		    pixel_number |= ((x >> 2) & 1) << 3; // pn[3] = x[2]
		    pixel_number |= ((y >> 1) & 1) << 4; // pn[4] = y[1]
		    pixel_number |= ((y >> 2) & 1) << 5; // pn[5] = y[2]
		    break;
	    }
        return pixel_number;
}

static inline GLint r600_1d_tile_helper(const struct radeon_renderbuffer * rrb,
					GLint x, GLint y, GLint is_depth, GLint is_stencil)
{
    GLint element_bytes = rrb->cpp;
    GLint num_samples = 1;
    GLint tile_width = 8;
    GLint tile_height = 8;
    GLint tile_thickness = 1;
    GLint pitch_elements = rrb->pitch / element_bytes;
    GLint height = rrb->base.Height;
    GLint z = 0;
    GLint sample_number = 0;
    /* */
    GLint tile_bytes;
    GLint tiles_per_row;
    GLint tiles_per_slice;
    GLint slice_offset;
    GLint tile_row_index;
    GLint tile_column_index;
    GLint tile_offset;
    GLint pixel_number = 0;
    GLint element_offset;
    GLint offset = 0;

    tile_bytes = tile_width * tile_height * tile_thickness * element_bytes * num_samples;
    tiles_per_row = pitch_elements / tile_width;
    tiles_per_slice = tiles_per_row * (height / tile_height);
    slice_offset = (z / tile_thickness) * tiles_per_slice * tile_bytes;
    tile_row_index = y / tile_height;
    tile_column_index = x / tile_width;
    tile_offset = ((tile_row_index * tiles_per_row) + tile_column_index) * tile_bytes;

    if (is_depth) {
	    GLint pixel_offset = 0;

	    pixel_number |= ((x >> 0) & 1) << 0; // pn[0] = x[0]
	    pixel_number |= ((y >> 0) & 1) << 1; // pn[1] = y[0]
	    pixel_number |= ((x >> 1) & 1) << 2; // pn[2] = x[1]
	    pixel_number |= ((y >> 1) & 1) << 3; // pn[3] = y[1]
	    pixel_number |= ((x >> 2) & 1) << 4; // pn[4] = x[2]
	    pixel_number |= ((y >> 2) & 1) << 5; // pn[5] = y[2]
	    switch (element_bytes) {
	    case 2:
		    pixel_offset = pixel_number * element_bytes * num_samples;
		    break;
	    case 4:
		    /* stencil and depth data are stored separately within a tile.
		     * stencil is stored in a contiguous tile before the depth tile.
		     * stencil element is 1 byte, depth element is 3 bytes.
		     * stencil tile is 64 bytes.
		     */
		    if (is_stencil)
			    pixel_offset = pixel_number * 1 * num_samples;
		    else
			    pixel_offset = (pixel_number * 3 * num_samples) + 64;
		    break;
	    }
	    element_offset = pixel_offset + (sample_number * element_bytes);
    } else {
	    GLint sample_offset;

        pixel_number = r600_coord_within_microtile(x, y, element_bytes);
	    sample_offset = sample_number * (tile_bytes / num_samples);
	    element_offset = sample_offset + (pixel_number * element_bytes);
    }
    offset = slice_offset + tile_offset + element_offset;
    return offset;
}

static inline GLint r600_log2(GLint n)
{
	GLint log2 = 0;

	while (n >>= 1)
		++log2;
	return log2;
}

static inline GLint r600_2d_tile_helper(const struct radeon_renderbuffer * rrb,
					GLint x, GLint y, GLint is_depth, GLint is_stencil)
{
	GLint group_bytes = rrb->group_bytes;
	GLint num_channels = rrb->num_channels;
	GLint num_banks = rrb->num_banks;
	GLint r7xx_bank_op = rrb->r7xx_bank_op;
	/* */
	GLint group_bits = r600_log2(group_bytes);
	GLint channel_bits = r600_log2(num_channels);
	GLint bank_bits = r600_log2(num_banks);
	GLint element_bytes = rrb->cpp;
	GLint num_samples = 1;
	GLint tile_width = 8;
	GLint tile_height = 8;
	GLint tile_thickness = 1;
	GLint macro_tile_width = num_banks;
	GLint macro_tile_height = num_channels;
	GLint pitch_elements = (rrb->pitch / element_bytes) / tile_width;
	GLint height = rrb->base.Height / tile_height;
	GLint z = 0;
	GLint sample_number = 0;
	/* */
	GLint tile_bytes;
	GLint macro_tile_bytes;
	GLint macro_tiles_per_row;
	GLint macro_tiles_per_slice;
	GLint slice_offset;
	GLint macro_tile_row_index;
	GLint macro_tile_column_index;
	GLint macro_tile_offset;
	GLint pixel_number = 0;
	GLint element_offset;
	GLint bank = 0;
	GLint channel = 0;
	GLint total_offset;
	GLint group_mask = (1 << group_bits) - 1;
	GLint offset_low;
	GLint offset_high;
	GLint offset = 0;

	switch (num_channels) {
	case 2:
	default:
		// channel[0] = x[3] ^ y[3]
		channel |= (((x >> 3) ^ (y >> 3)) & 1) << 0;
		break;
	case 4:
		// channel[0] = x[4] ^ y[3]
		channel |= (((x >> 4) ^ (y >> 3)) & 1) << 0;
		// channel[1] = x[3] ^ y[4]
		channel |= (((x >> 3) ^ (y >> 4)) & 1) << 1;
		break;
	case 8:
		// channel[0] = x[5] ^ y[3]
		channel |= (((x >> 5) ^ (y >> 3)) & 1) << 0;
		// channel[0] = x[4] ^ x[5] ^ y[4]
		channel |= (((x >> 4) ^ (x >> 5) ^ (y >> 4)) & 1) << 1;
		// channel[0] = x[3] ^ y[5]
		channel |= (((x >> 3) ^ (y >> 5)) & 1) << 2;
		break;
	}

	switch (num_banks) {
	case 4:
		// bank[0] = x[3] ^ y[4 + log2(num_channels)]
		bank |= (((x >> 3) ^ (y >> (4 + channel_bits))) & 1) << 0;
		if (r7xx_bank_op)
			// bank[1] = x[3] ^ y[4 + log2(num_channels)] ^ x[5]
			bank |= (((x >> 4) ^ (y >> (3 + channel_bits)) ^ (x >> 5)) & 1) << 1;
		else
			// bank[1] = x[4] ^ y[3 + log2(num_channels)]
			bank |= (((x >> 4) ^ (y >> (3 + channel_bits))) & 1) << 1;
		break;
	case 8:
		// bank[0] = x[3] ^ y[5 + log2(num_channels)]
		bank |= (((x >> 3) ^ (y >> (5 + channel_bits))) & 1) << 0;
		// bank[1] = x[4] ^ y[4 + log2(num_channels)] ^ y[5 + log2(num_channels)]
		bank |= (((x >> 4) ^ (y >> (4 + channel_bits)) ^ (y >> (5 + channel_bits))) & 1) << 1;
		if (r7xx_bank_op)
			// bank[2] = x[5] ^ y[3 + log2(num_channels)] ^ x[6]
			bank |= (((x >> 5) ^ (y >> (3 + channel_bits)) ^ (x >> 6)) & 1) << 2;
		else
			// bank[2] = x[5] ^ y[3 + log2(num_channels)]
			bank |= (((x >> 5) ^ (y >> (3 + channel_bits))) & 1) << 2;
		break;
	}

	tile_bytes = tile_width * tile_height * tile_thickness * element_bytes * num_samples;
	macro_tile_bytes = macro_tile_width * macro_tile_height * tile_bytes;
	macro_tiles_per_row = pitch_elements / macro_tile_width;
	macro_tiles_per_slice = macro_tiles_per_row * (height / macro_tile_height);
	slice_offset = (z / tile_thickness) * macro_tiles_per_slice * macro_tile_bytes;
	macro_tile_row_index = (y / tile_height) / macro_tile_height;
	macro_tile_column_index = (x / tile_width) / macro_tile_width;
	macro_tile_offset = ((macro_tile_row_index * macro_tiles_per_row) + macro_tile_column_index) * macro_tile_bytes;

	if (is_depth) {
		GLint pixel_offset = 0;

		pixel_number |= ((x >> 0) & 1) << 0; // pn[0] = x[0]
		pixel_number |= ((y >> 0) & 1) << 1; // pn[1] = y[0]
		pixel_number |= ((x >> 1) & 1) << 2; // pn[2] = x[1]
		pixel_number |= ((y >> 1) & 1) << 3; // pn[3] = y[1]
		pixel_number |= ((x >> 2) & 1) << 4; // pn[4] = x[2]
		pixel_number |= ((y >> 2) & 1) << 5; // pn[5] = y[2]
		switch (element_bytes) {
		case 2:
			pixel_offset = pixel_number * element_bytes * num_samples;
			break;
		case 4:
			/* stencil and depth data are stored separately within a tile.
			 * stencil is stored in a contiguous tile before the depth tile.
			 * stencil element is 1 byte, depth element is 3 bytes.
			 * stencil tile is 64 bytes.
			 */
			if (is_stencil)
				pixel_offset = pixel_number * 1 * num_samples;
			else
				pixel_offset = (pixel_number * 3 * num_samples) + 64;
			break;
		}
		element_offset = pixel_offset + (sample_number * element_bytes);
	} else {
		GLint sample_offset;

        pixel_number = r600_coord_within_microtile(x, y, element_bytes);

		sample_offset = sample_number * (tile_bytes / num_samples);
		element_offset = sample_offset + (pixel_number * element_bytes);
	}
	total_offset = (slice_offset + macro_tile_offset) >> (channel_bits + bank_bits);
	total_offset += element_offset;

	offset_low = total_offset & group_mask;
	offset_high = (total_offset & ~group_mask) << (channel_bits + bank_bits);
	offset = (bank << (group_bits + channel_bits)) + (channel << group_bits) + offset_low + offset_high;

	return offset;
}

// end Mesa code

#pragma pack(1)

struct DdsPixelFormat {
	uint32_t	dwSize;
	uint32_t	dwFlags;
	uint32_t	dwFourCC;
	uint32_t	dwRGBBitCount;
	uint32_t	dwRBitMask;
	uint32_t	dwGBitMask;
	uint32_t	dwBBitMask;
	uint32_t	dwABitMask;
};

struct DdsHeader {
	uint32_t	dwSize;
	uint32_t	dwFlags;
	uint32_t	dwHeight;
	uint32_t	dwWidth;
	uint32_t	dwPitchOrLinearSize;
	uint32_t	dwDepth;
	uint32_t	dwMipMapCount;
	uint32_t	dwReserved1[11];
	DdsPixelFormat	ddspf;
	uint32_t	dwCaps;
	uint32_t	dwCaps2;
	uint32_t	dwCaps3;
	uint32_t	dwCaps4;
	uint32_t	dwReserved2;
};
static_assert(sizeof(DdsHeader) == 124, "dds header should be 124 bytes long");
#pragma pack()

enum {
	ADDR_FMT_BC1 = 0x31,
	ADDR_FMT_BC3 = 0x33,
};

int main(int argc, char** argv){
	int mode = ADDR_FMT_BC3; // DCT3
	int cpp = mode == ADDR_FMT_BC1? 8: 16;
	int expandX = 4;
	int expandY = 4;
	int width = 128;//128;
	int height = 128;//128;
	int bufSize = (width/expandX)*(height/expandY)*cpp;
	unsigned char* inBuf = new unsigned char[bufSize];
	unsigned char* outBuf = new unsigned char[bufSize];
	unsigned char* usedBuf = new unsigned char[bufSize];
	FILE* f = fopen(argv[1], "rb");
	fread(inBuf, bufSize, 1, f);
	fclose(f);
	
	radeon_renderbuffer texture;
	texture.cpp = cpp; // compressed
	texture.base.Height = height / expandY;
	texture.pitch = (width / expandX) * texture.cpp;
	texture.group_bytes = 256;
	texture.num_channels = 2;
	texture.num_banks = 4; // should be 8, but this works for some reason so who cares
	texture.r7xx_bank_op = 0;
	for (int y = 0; y < height / expandY; y++) {
		for (int x = 0; x < width / expandX; x++) {
			int offset = r600_2d_tile_helper(&texture, x, y, 0, 0);
			if (offset > bufSize) {
				fprintf(stderr, "offset is %x\n", offset);
				continue;
			}
			if (usedBuf[offset]) {
				fprintf(stderr, "already used %x\n", offset);
			}
			usedBuf[offset] = 1;
			//printf("%x %x offset %x real %x\n", x, y, offset, (y*texture.pitch+x*texture.cpp));
			memcpy(outBuf + (y*texture.pitch+x*texture.cpp), inBuf + offset, texture.cpp);
		}
	}

	DdsHeader ddsHeader;
	memset(&ddsHeader, 0, sizeof(ddsHeader));
	ddsHeader.dwSize = sizeof(ddsHeader);
	ddsHeader.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000 | 0x80000;
	ddsHeader.dwHeight = height;
	ddsHeader.dwWidth = width;
	ddsHeader.dwPitchOrLinearSize = bufSize;
	ddsHeader.ddspf.dwSize = sizeof(ddsHeader.ddspf);
	ddsHeader.ddspf.dwFlags = 0x1 | 0x4;
	//ddsHeader.ddspf.dwFourCC = '5TXD';
	ddsHeader.ddspf.dwFourCC = (mode == ADDR_FMT_BC1)? '1TXD': '5TXD';
	ddsHeader.dwCaps = 0x1000;
	FILE* f2 = fopen(argv[2], "wb");
	fwrite("DDS ", 4, 1, f2);
	fwrite(&ddsHeader, sizeof(ddsHeader), 1, f2);
	fwrite(outBuf, bufSize, 1, f2);
	fclose(f2);

}