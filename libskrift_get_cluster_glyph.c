/* See LICENSE file for copyright and license details. */
#include "common.h"

/* TODO try normalisation if not found, then try fallback fonts */

ssize_t
libskrift_get_cluster_glyph(LIBSKRIFT_CONTEXT *ctx, const char *text, size_t text_length,
                            struct libskrift_saved_grapheme *saved, double x, double y, struct libskrift_glyph **glyphp)
{
	struct libskrift_glyph *glyph0, *glyph1, *glyph2;
	uint32_t cp0, cp1;
	int state = 0;
	size_t r, len;

	*glyphp = NULL;

	if (!text_length) {
		errno = EINVAL;
		return -1;
	}

	if (saved && saved->have_saved) {
		cp0 = saved->cp;
		len = saved->len;
	} else {
		len = grapheme_cp_decode(&cp0, (const void *)text, text_length);
	}

	if (libskrift_get_grapheme_glyph(ctx, cp0, x, y, &glyph0))
		return -1;

	x += glyph0->advance * ctx->char_x_advancement;
	y += glyph0->advance * ctx->char_y_advancement;
	for (; len < text_length; cp0 = cp1, len += r) {
		r = grapheme_cp_decode(&cp1, (const void *)&text[len], text_length - len);
		if (grapheme_boundary(cp0, cp1, &state)) {
			if (saved) {
				saved->have_saved = 1;
				saved->cp = cp1;
				saved->len = r;
			}
			goto out;
		}

		if (libskrift_get_grapheme_glyph(ctx, cp1, x, y, &glyph1)) {
			free(glyph0);
			return -1;
		}
		x += glyph1->advance * ctx->char_x_advancement;
		y += glyph1->advance * ctx->char_y_advancement;

		if (libskrift_merge_glyphs(ctx, glyph0, glyph1, &glyph2)) {
			free(glyph0);
			free(glyph1);
			return -1;
		}

		free(glyph0);
		free(glyph1);
		glyph0 = glyph2;
	}
	if (saved) {
		saved->have_saved = 0;
		saved->cp = 0;
		saved->len = 0;
	}

out:
	*glyphp = glyph0;

	return (ssize_t)len;
}
