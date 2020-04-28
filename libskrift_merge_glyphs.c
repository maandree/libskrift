/* See LICENSE file for copyright and license details. */
#include "common.h"

/* TODO How common are grapheme clusters with more than two glyphs? Should we need a variadic version? */

int
libskrift_merge_glyphs(LIBSKRIFT_CONTEXT *ctx, const struct libskrift_glyph *glyph1,
                       const struct libskrift_glyph *glyph2, struct libskrift_glyph **glyphp)
{
	int16_t x1a, x1b, x2a, x2b, y1a, y1b, y2a, y2b, x1, x2, y1, y2;
	size_t width, height, r, c, size, psize;
	size_t src_off, dest_off, src_linesize, dest_linesize;

	psize = ctx->rendering.smoothing ? 3 : 1;

	x1a = glyph1->x;
	x1b = glyph2->x;
	y1a = glyph1->y;
	y1b = glyph2->y;

	x2a = (int16_t)(x1a + (int16_t)glyph1->width);
	x2b = (int16_t)(x1b + (int16_t)glyph2->width);
	y2a = (int16_t)(y1a + (int16_t)glyph1->height);
	y2b = (int16_t)(y1b + (int16_t)glyph2->height);

	x1 = MIN(x1a, x1b);
	y1 = MIN(y1a, y1b);
	x2 = MAX(x2a, x2b);
	y2 = MAX(y2a, y2b);

	size = psize;
	size *= width  = (uint16_t)(x2 - x1);
	size *= height = (uint16_t)(y2 - y1);

	*glyphp = calloc(1, offsetof(struct libskrift_glyph, image) + size);
	if (!*glyphp)
		return -1;

	(*glyphp)->advance = glyph1->advance + glyph2->advance;
	(*glyphp)->x       = x1;
	(*glyphp)->y       = y1;
	(*glyphp)->width   = (uint16_t)width;
	(*glyphp)->height  = (uint16_t)height;
	(*glyphp)->size    = size;

	dest_linesize = width * psize;

	src_linesize = glyph1->width * psize;
	dest_off  = (size_t)(glyph1->y - y1) * dest_linesize;
	dest_off += (size_t)(glyph1->x - x1) * psize;
	for (r = src_off = 0; r < glyph1->height; r++, dest_off += dest_linesize, src_off += src_linesize)
		memcpy(&(*glyphp)->image[dest_off], &glyph1->image[src_off], src_linesize);

	src_linesize = glyph2->width * psize;
	dest_off  = (size_t)(glyph2->y - y1) * dest_linesize;
	dest_off += (size_t)(glyph2->x - x1) * psize;

	/* TODO only use merging on actual overlap */
#ifndef OR_MERGE
	if (ctx->rendering.smoothing) {
#ifdef SUM_MERGE
		unsigned sum;
		for (r = src_off = 0; r < glyph2->height; r++, dest_off += dest_linesize, src_off += src_linesize) {
			for (c = 0; c < src_linesize; c++) {
				sum = (unsigned)(*glyphp)->image[dest_off + c] + (unsigned)glyph2->image[src_off + c];
				(*glyphp)->image[dest_off + c] = (uint8_t)(sum | ((sum & 0x100) - 1));
			}
		}
#else
		for (r = src_off = 0; r < glyph2->height; r++, dest_off += dest_linesize, src_off += src_linesize)
			for (c = 0; c < src_linesize; c++)
				(*glyphp)->image[dest_off + c] = MAX((*glyphp)->image[dest_off + c], glyph2->image[src_off + c]);
#endif
	} else {
#endif
		for (r = src_off = 0; r < glyph2->height; r++, dest_off += dest_linesize, src_off += src_linesize)
			for (c = 0; c < src_linesize; c++)
				(*glyphp)->image[dest_off + c] |= glyph2->image[src_off + c];
#ifndef OR_MERGE
	}
#endif

	return 0;
}
