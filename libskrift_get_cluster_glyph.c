/* See LICENSE file for copyright and license details. */
#include "common.h"

/* TODO try normalisation if not found, then try fallback fonts */

ssize_t
libskrift_get_cluster_glyph(LIBSKRIFT_CONTEXT *ctx, const char *text, struct libskrift_saved_grapheme *saved,
                            double x, double y, struct libskrift_glyph **glyphp)
{
	struct libskrift_glyph *glyph0, *glyph1, *glyph2;
	uint32_t cp0, cp1;
	int state = 0;
	size_t r, len;

	*glyphp = NULL;

	if (saved && saved->cp) {
		cp0 = saved->cp;
		len = saved->len;
	} else {
		len = grapheme_decode(text, &cp0);
		if (!len) {
			errno = EINVAL;
			return -1;
		}
	}

	if (libskrift_get_grapheme_glyph(ctx, cp0, x, y, &glyph0))
		return -1;

	x += glyph0->advance;
	for (; cp0; cp0 = cp1, len += r) {
		r = grapheme_decode(&text[len], &cp1);
		if (!r) {
			if (saved) {
				saved->cp = 0;
				saved->len = 0;
			}
			break;
		}
		if (grapheme_boundary(cp0, cp1, &state)) {
			if (saved) {
				saved->cp = cp1;
				saved->len = r;
			}
			break;
		}

		if (libskrift_get_grapheme_glyph(ctx, cp1, x, y, &glyph1)) {
			free(glyph0);
			return -1;
		}
		x += glyph1->advance;

		if (libskrift_merge_glyphs(ctx, glyph0, glyph1, &glyph2)) {
			free(glyph0);
			free(glyph1);
			return -1;
		}

		free(glyph0);
		free(glyph1);
		glyph0 = glyph2;
	}

	*glyphp = glyph0;

	return (ssize_t)len;
}
