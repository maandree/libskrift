/* See LICENSE file for copyright and license details. */
#include "libskrift.h"

#include <stdlib.h>
#include <string.h>

#include <grapheme.h>

int
main(void)
{
	LIBSKRIFT_FONT *font;
	LIBSKRIFT_CONTEXT *ctx;
	struct libskrift_glyph *glyph;
	struct libskrift_rendering rendering = LIBSKRIFT_DEFAULT_RENDERING;
	uint16_t i, x, y;
	double height;

	rendering.smoothing      = LIBSKRIFT_SUBPIXEL;
	rendering.subpixel_order = LIBSKRIFT_RGB;
	rendering.flags          = 0;

	if (libskrift_open_font_file(&font, DEMO_FONT)) {
		perror("libskrift_open_font_file");
		return -1;
	}
	height = libskrift_points_to_pixels(72, &rendering);
	if (libskrift_create_context(&ctx, &font, 1, height, &rendering, NULL)) {
		perror("libskrift_create_context");
		return -1;
	}
	libskrift_close_font(font);

#if 1
	if (libskrift_get_cluster_glyph(ctx, "x̴̑", NULL, 0, 0, &glyph) < 0) {
		perror("libskrift_get_cluster_glyph");
		return -1;
	}
#else
	if (libskrift_get_grapheme_glyph(ctx, 197 /* Å */, 0, 0, &glyph)) {
		perror("libskrift_get_grapheme_glyph");
		return -1;
	}
#endif

	if (rendering.smoothing == LIBSKRIFT_GREYSCALE) {
		printf("P2\n%u %u\n255\n", glyph->width, glyph->height);
		printf("# x-position: %i\n",  glyph->x);
		printf("# y-position: %i\n",  glyph->y);
		printf("# advance:    %lf\n", glyph->advance);
		for (i = y = 0; y < glyph->height; y++) {
			for (x = 0; x < glyph->width; x++, i++)
				printf("%3u ", glyph->image[i]);
			printf("\n\n");
		}
		fflush(stdout);
	} else {
		printf("P3\n%u %u\n255\n", glyph->width, glyph->height);
		printf("# x-position: %i\n",  glyph->x);
		printf("# y-position: %i\n",  glyph->y);
		printf("# advance:    %lf\n", glyph->advance);
		for (i = y = 0; y < glyph->height; y++) {
			for (x = 0; x < glyph->width * 3; x++, i++)
				printf("%3u ", glyph->image[i]);
			printf("\n\n");
		}
		fflush(stdout);
	}

	free(glyph);
	libskrift_free_context(ctx);
	return 0;
}
