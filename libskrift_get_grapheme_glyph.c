/* See LICENSE file for copyright and license details. */
#include "common.h"

static int
get_grapheme_glyph_with_font(LIBSKRIFT_CONTEXT *ctx, libskrift_codepoint_t codepoint, double cursor_x,
                             double cursor_y, struct libskrift_glyph **glyphp, LIBSKRIFT_FONT *font)
{
	struct SFT_Char sft_chr;
	struct SFT sft_ctx;
	size_t size = 1, width1, width2, width3, osize, off, r, c, i;
	int top = 0, left = 0, right = 0, bottom = 0;
	uint16_t width, height, vmul = 1, hmul = 1;
	uint8_t *image, *in_image;

	memset(&sft_chr, 0, sizeof(sft_chr));
	sft_ctx = ctx->schrift_ctx;

	sft_ctx.font = font->font;
	sft_ctx.x = cursor_x * (ctx->subpixel_horizontally ? 3 : 1);
	sft_ctx.y = cursor_y * (ctx->subpixel_vertically ? 3 : 1);
	sft_ctx.flags = SFT_DOWNWARD_Y | SFT_CHAR_IMAGE;

	errno = 0;
	if (sft_char(&sft_ctx, codepoint, &sft_chr))
		return errno ? -1 : 1;

	if (ctx->subpixel_horizontally) {
		hmul   = 3;
		size   = 3;
		left   = (3 + sft_chr.x % 3) % 3;
		right  = (3 - (sft_chr.width + left) % 3) % 3;
	} else if (ctx->subpixel_vertically) {
		vmul   = 3;
		size   = 3;
		top    = (3 + sft_chr.y % 3) % 3;
		bottom = (3 - (sft_chr.height + top) % 3) % 3;
	}

	width  = (uint16_t)(sft_chr.width  + left + right) / hmul;
	height = (uint16_t)(sft_chr.height + top + bottom) / vmul;
	size  *= (size_t)width * (size_t)height;

	*glyphp = malloc(FLEXSTRUCTSIZE(struct libskrift_glyph, image, size));
	if (!*glyphp) {
		free(sft_chr.image);
		return -1;
	}

	(*glyphp)->advance = sft_chr.advance / hmul;
	(*glyphp)->x       = (int16_t)((sft_chr.x - left) / (int16_t)hmul);
	(*glyphp)->y       = (int16_t)((sft_chr.y - top)  / (int16_t)vmul);
	(*glyphp)->width   = width;
	(*glyphp)->height  = height;
	(*glyphp)->size    = size;

	image = (*glyphp)->image;
	in_image = sft_chr.image;
	if (ctx->subpixel_horizontally && width) {
		width3 = (size_t)width * 3;
		for (r = 0; r < height; r++) {
			image[0] = image[width3 - 3] = 0;
			image[1] = image[width3 - 2] = 0;
			image[2] = image[width3 - 1] = 0;
			memcpy(&image[left], in_image, (size_t)sft_chr.width);
			image    += width3;
			in_image += sft_chr.width;
		}
	} else if (ctx->subpixel_vertically && height) {
		width1 = (size_t)width * 1;
		width2 = (size_t)width * 2;
		width3 = (size_t)width * 3;
		in_image = realloc(in_image, size);
		if (!in_image) {
			free(sft_chr.image);
			free(*glyphp);
			*glyphp = NULL;
			return -1;
		}
		sft_chr.image = in_image;
		osize = (size_t)sft_chr.width * (size_t)sft_chr.height;
		off = (size_t)top;
		if (off) {
			off *= width;
			memmove(&in_image[off], in_image, osize);
			memset(in_image, 0, off);
			osize += off;
		}
		memset(&in_image[osize], 0, size - osize);
		for (i = r = 0; r < height; r++) {
			for (c = 0; c < width; c++, i += 3) {
				image[i + 0] = in_image[c];
				image[i + 1] = in_image[c + width1];
				image[i + 2] = in_image[c + width2];
			}
			in_image += width3;
		}
	} else {
		memcpy(image, in_image, size);
	}

	free(sft_chr.image);
	return 0;
}

int
libskrift_get_grapheme_glyph(LIBSKRIFT_CONTEXT *ctx, libskrift_codepoint_t codepoint,
                             double cursor_x, double cursor_y, struct libskrift_glyph **glyphp)
{
	size_t i;
	int r;

	for (i = 0; i < ctx->nfonts; i++) {
		r = get_grapheme_glyph_with_font(ctx, codepoint, cursor_x, cursor_y, glyphp, ctx->fonts[i]);
		if (r <= 0)
			return r;
	}

	/* TODO Use glyph 0 */
	return -1;
}
