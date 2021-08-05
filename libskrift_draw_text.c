/* See LICENSE file for copyright and license details. */
#include "common.h"

static void
reverse_text(const char *restrict text, size_t text_length, char *restrict s)
{
	size_t n, off = text_length;
	for (; text_length; text += n, text_length -= n) {
		off -= n = grapheme_bytelen(text);
		memcpy(&s[off], text, n);
	}
}

int
libskrift_draw_text(LIBSKRIFT_CONTEXT *ctx, const char *text, size_t text_length,
                    const struct libskrift_colour *colour, int16_t x, int16_t y, struct libskrift_image *image)
{
	struct libskrift_saved_grapheme saved = LIBSKRIFT_NO_SAVED_GRAPHEME;
	struct libskrift_glyph *glyph;
	char *buffer = NULL;
	double xpos = 0, ypos = 0;
	ssize_t len;
	int r;

	if (ctx->rendering.flags & LIBSKRIFT_MIRROR_TEXT) {
		if (text_length < 1024) {
			buffer = alloca(text_length + 1);
		} else {
			buffer = malloc(text_length + 1);
			if (!buffer)
				return -1;
		}
		reverse_text(text, text_length, buffer);
		text = buffer;
		if (text_length < 1024)
			buffer = NULL;
	}

	for (; text_length; text += len, text_length -= (size_t)len) {
		len = libskrift_get_cluster_glyph(ctx, text, text_length, &saved, xpos, ypos, &glyph);
		if (len < 0) {
			free(buffer);
			return -1;
		}

		r = libskrift_apply_glyph(ctx, glyph, colour, x, y, image);
		xpos += (glyph->advance + ctx->rendering.interletter_spacing) * ctx->x_advancement;
		ypos += (glyph->advance + ctx->rendering.interletter_spacing) * ctx->y_advancement;
		free(glyph);
		if (r) {
			free(buffer);
			return -1;
		}
	}

	free(buffer);
	return 0;
}
