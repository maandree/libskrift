/* See LICENSE file for copyright and license details. */
#include "common.h"

static const uint8_t gamma_map[] = {
	  0,  13,  22,  28,  34,  38,  42,  46,  50,  53,  56,  59,  61,  64,  66,  69,
	 71,  73,  75,  77,  79,  81,  83,  85,  86,  88,  90,  92,  93,  95,  96,  98,
	 99, 101, 102, 104, 105, 106, 108, 109, 110, 112, 113, 114, 115, 117, 118, 119,
	120, 121, 122, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
	137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 148, 149, 150, 151,
	152, 153, 154, 155, 155, 156, 157, 158, 159, 159, 160, 161, 162, 163, 163, 164,
	165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 175, 176,
	177, 178, 178, 179, 180, 180, 181, 182, 182, 183, 184, 185, 185, 186, 187, 187,
	188, 189, 189, 190, 190, 191, 192, 192, 193, 194, 194, 195, 196, 196, 197, 197,
	198, 199, 199, 200, 200, 201, 202, 202, 203, 203, 204, 205, 205, 206, 206, 207,
	208, 208, 209, 209, 210, 210, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216,
	216, 217, 218, 218, 219, 219, 220, 220, 221, 221, 222, 222, 223, 223, 224, 224,
	225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 232, 232, 233,
	233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238, 238, 239, 239, 240, 240,
	241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 246, 247, 247, 248,
	248, 249, 249, 250, 250, 251, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255
};

int
libskrift_get_grapheme_glyph(LIBSKRIFT_CONTEXT *ctx, libskrift_codepoint_t codepoint,
                             double cursor_x, double cursor_y, struct libskrift_glyph **glyphp)
{
	struct SFT_Char sft_chr;
	size_t size = 1, width0, width1, width2, width3, osize, off, r, c, i;
	int top = 0, left = 0, right = 0, bottom = 0;
	uint16_t width, height, vmul = 1, hmul = 1;
	uint8_t *image, *in_image, t;

	memset(&sft_chr, 0, sizeof(sft_chr));

	ctx->schrift_ctx.x = cursor_x * (ctx->subpixel_horizontally ? 3 : 1);
	ctx->schrift_ctx.y = cursor_y * (ctx->subpixel_vertically ? 3 : 1);
	ctx->schrift_ctx.flags = SFT_DOWNWARD_Y | SFT_CHAR_IMAGE;

	if (sft_char(&ctx->schrift_ctx, codepoint, &sft_chr))
		return -1;

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

	*glyphp = malloc(offsetof(struct libskrift_glyph, image) + size);
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
		if (ctx->subpixel_bgr) {
			image = (*glyphp)->image;
			for (i = 0; i < size; i += 3) {
				t = image[i + 0];
				image[i + 0] = image[i + 2];
				image[i + 2] = t;
			}
		}
	} else if (ctx->subpixel_vertically && height) {
		width0 = (size_t)width * (ctx->subpixel_bgr ? 2 : 0);
		width1 = (size_t)width * 1;
		width2 = (size_t)width * (ctx->subpixel_bgr ? 0 : 2);
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
				image[i + 0] = in_image[c + width0];
				image[i + 1] = in_image[c + width1];
				image[i + 2] = in_image[c + width2];
			}
			in_image += width3;
		}
	} else {
		memcpy(image, in_image, size);
	}

	if (ctx->rendering.flags & LIBSKRIFT_CORRECT_GAMMA) {
		image = (*glyphp)->image;
		for (i = 0; i < size; i++)
			image[i] = gamma_map[image[i]];
	}

	free(sft_chr.image);
	return 0;
}
