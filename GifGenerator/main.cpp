
#include "gif.h"
#include "DIBReader.h"

#define _CRT_SECURE_NO_WARNINGS

int main()
{
	char *filename = "e:\\image\\11.bmp";

	FILE *stream = fopen(filename, "rb");

	char *gifname = "e:\\image\\12.gif";

	if (stream)
	{
		bmp_header_t header;
		bmp_dib_header_t dib_header;
		if (bmp_read_header(stream, &header) && bmp_read_dib_header(stream, &dib_header))
		{
			size_t buffer_size;
			auto pixel_data = bmp_read_pixel_data(stream, header.offset, &dib_header, &buffer_size);
			fclose(stream);

			if (pixel_data)
			{
				GifWriter gif_writer;
				GifBegin(&gif_writer, gifname, dib_header.width, dib_header.height, 500);
				GifWriteFrame(&gif_writer, (const uint8_t*)pixel_data, dib_header.width, dib_header.height, 500);
				GifEnd(&gif_writer);
			}
		}
	}

	printf("Gif generator finished.\n");
	getchar();
	return 0;
}