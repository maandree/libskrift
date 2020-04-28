/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_draw_text(LIBSKRIFT_CONTEXT *ctx, const char *text, const struct libskrift_colour *colour,
                    int16_t x, int16_t y, struct libskrift_image *image)
{
	struct libskrift_saved_grapheme saved = LIBSKRIFT_NO_SAVED_GRAPHEME;
	struct libskrift_glyph *glyph;
	double xpos = 0;
	ssize_t len;
	int r;

	for (; *text; text += len) {
		len = libskrift_get_cluster_glyph(ctx, text, &saved, xpos, 0, &glyph);
		if (len < 0)
			return -1;

		r = libskrift_apply_glyph(ctx, glyph, colour, x, y, image);
		xpos += glyph->advance;
		free(glyph);
		if (r)
			return -1;
	}

	return 0;
}
