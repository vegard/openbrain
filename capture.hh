#ifndef CAPTURE_HH
#define CAPTURE_HH

extern "C" {
#include <png.h>
}

#ifdef CONFIG_CAPTURE
static void
capture()
{
	static const unsigned int width = 640;
	static const unsigned int height = 480;

	static unsigned int frame = 0;

	++frame;
#ifdef CONFIG_FRAMES
	if (frame == CONFIG_FRAMES)
		exit(0);
#endif

	printf("\e[Lframe %d\r", frame);
	fflush(stdout);

	static unsigned short capture[width * height * 3];
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_SHORT, capture);

	static char fn[32];
	snprintf(fn, sizeof(fn), "png/%04d.png", frame - 1);

	FILE *fp = fopen(fn, "wb");
	if(!fp) {
		fprintf(stderr, "error: fopen\n");
		exit(1);
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);
	if(!png_ptr) {
		fprintf(stderr, "error: png_create_write_struct\n");
		exit(1);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		fprintf(stderr, "error: png_create_info_struct\n");
		exit(1);
	}

	png_init_io(png_ptr, fp);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_IHDR(png_ptr, info_ptr, width, height, 16, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_bytep rows[height];

	for(unsigned int i = 0; i < height; i++)
		rows[i] = (png_bytep) &capture[(height - i - 1) * width * 3];

	png_set_rows(png_ptr, info_ptr, rows);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	fclose(fp);
}
#endif

#endif
