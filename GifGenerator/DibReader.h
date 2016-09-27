#pragma once

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef struct {
	uint8_t magic[2];   /* the magic number used to identify the BMP file:
						0x42 0x4D (Hex code points for B and M).
						The following entries are possible:
						BM - Windows 3.1x, 95, NT, ... etc
						BA - OS/2 Bitmap Array
						CI - OS/2 Color Icon
						CP - OS/2 Color Pointer
						IC - OS/2 Icon
						PT - OS/2 Pointer. */
	uint32_t filesz;    /* the size of the BMP file in bytes */
	uint32_t reserved;  /* reserved. */
	uint32_t offset;    /* the offset, i.e. starting address,
						of the byte where the bitmap data can be found. */
} bmp_header_t;

typedef struct {
	uint32_t header_sz;     /* the size of this header (40 bytes) */
	uint32_t width;         /* the bitmap width in pixels */
	uint32_t height;        /* the bitmap height in pixels */
	uint16_t nplanes;       /* the number of color planes being used.
							Must be set to 1. */
	uint16_t depth;         /* the number of bits per pixel,
							which is the color depth of the image.
							Typical values are 1, 4, 8, 16, 24 and 32. */
	uint32_t compress_type; /* the compression method being used.
							See also bmp_compression_method_t. */
	uint32_t bmp_bytesz;    /* the image size. This is the size of the raw bitmap
							data (see below), and should not be confused
							with the file size. */
	uint32_t hres;          /* the horizontal resolution of the image.
							(pixel per meter) */
	uint32_t vres;          /* the vertical resolution of the image.
							(pixel per meter) */
	uint32_t ncolors;       /* the number of colors in the color palette,
							or 0 to default to 2<sup><i>n</i></sup>. */
	uint32_t nimpcolors;    /* the number of important colors used,
							or 0 when every color is important;
							generally ignored. */
} bmp_dib_header_t;


bool bmp_read_header(FILE *fp, bmp_header_t *header)
{
	assert(fp);
	assert(header);

	return fread(&(header->magic), sizeof(header->magic), 1, fp) &&
		fread(&(header->filesz), sizeof(uint32_t), 1, fp) &&
		fread(&(header->reserved), sizeof(uint32_t), 1, fp) &&
		fread(&(header->offset), sizeof(uint32_t), 1, fp) &&
		header->magic[0] == 0x42 && header->magic[1] == 0x4D;
}


bool bmp_read_dib_header(FILE *fp, bmp_dib_header_t *header)
{
	assert(fp);
	assert(header);

	return fread(&(header->header_sz), sizeof(uint32_t), 1, fp) &&
		fread(&(header->width), sizeof(uint32_t), 1, fp) &&
		fread(&(header->height), sizeof(uint32_t), 1, fp) &&
		fread(&(header->nplanes), sizeof(uint16_t), 1, fp) &&
		fread(&(header->depth), sizeof(uint16_t), 1, fp);
}


void *bmp_read_pixel_data(FILE *fp, uint32_t offset, const bmp_dib_header_t *header, size_t *buffer_size)
{
	assert(fp);
	assert(header);
	assert(buffer_size);

	if (fseek(fp, offset, SEEK_SET))
		return NULL;

	*buffer_size = header->height * header->width * 3;
	void *buffer = malloc(*buffer_size);
	if (buffer)
	{
		if (!fread(buffer, 1, *buffer_size, fp))
		{
			free(buffer);
			buffer = NULL;
		}
	}

	return buffer;
}
