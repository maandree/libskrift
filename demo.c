/* See LICENSE file for copyright and license details. */
#include "libskrift.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TEST_TEXT "hello world"

int
main(void)
{
	LIBSKRIFT_FONT *font;
	LIBSKRIFT_CONTEXT *ctx;
	struct libskrift_image image = {LIBSKRIFT_R8G8B8A8, LIBSKRIFT_HOST_SUBPIXEL, 0, 800, 600, NULL, NULL, NULL};
	struct libskrift_rendering rendering = LIBSKRIFT_DEFAULT_RENDERING;
	struct libskrift_colour colour = LIBSKRIFT_PREMULTIPLY(.80f, .50f, .80f, .50f, .20f);
	double height;
	size_t size, i;

	if (isatty(STDOUT_FILENO)) {
		fprintf(stderr, "Output file is a binary PAM image file, I am not writing that to a terminal.\n");
		return 1;
	}

	rendering.smoothing           = LIBSKRIFT_SUBPIXEL;
	rendering.subpixel_order      = LIBSKRIFT_NONE;
	rendering.flags               = LIBSKRIFT_MIRROR_CHARS | LIBSKRIFT_MIRROR_TEXT;
	rendering.interletter_spacing = 2;

	if (libskrift_open_font_file(&font, DEMO_FONT)) {
		perror("libskrift_open_font_file");
		return -1;
	}
	libskrift_add_rotation_degrees(rendering.text_transformation, -10);
	libskrift_add_shear(rendering.char_transformation, .75f, 0);
	height = libskrift_points_to_pixels(72, &rendering);
	if (libskrift_create_context(&ctx, &font, 1, height, &rendering, NULL)) {
		perror("libskrift_create_context");
		return -1;
	}
	libskrift_close_font(font);

	size = 4;
	size *= (size_t)image.width;
	size *= (size_t)image.height;
	image.image = malloc(size);
	if (!image.image) {
		perror("malloc");
		return -1;
	}
	for (i = 0; i < size; i += 4) {
		((uint8_t *)image.image)[i + 0] = 32U;
		((uint8_t *)image.image)[i + 1] = 48U;
		((uint8_t *)image.image)[i + 2] = 64U;
		((uint8_t *)image.image)[i + 3] = 250U;
	}

	if (libskrift_draw_text(ctx, TEST_TEXT, sizeof(TEST_TEXT) - 1, &colour, 0, 300, &image) < 0) {
		perror("libskrift_draw_text");
		return -1;
	}

	printf("P7\n");
	printf("WIDTH %u\n", image.width);
	printf("HEIGHT %u\n", image.height);
	printf("DEPTH 4\n");
	printf("MAXVAL 255\n");
	printf("TUPLTYPE RGB_ALPHA\n");
	printf("ENDHDR\n");
	fwrite(image.image, 1, size, stdout);
	fflush(stdout);

	free(image.image);
	libskrift_free_context(ctx);
	return 0;
}
