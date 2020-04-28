/* See LICENSE file for copyright and license details. */
#include "common.h"

static const float uint8_rmap[] = {
	  0/255.f,   1/255.f,   2/255.f,   3/255.f,   4/255.f,   5/255.f,   6/255.f,   7/255.f,
	  8/255.f,   9/255.f,  10/255.f,  11/255.f,  12/255.f,  13/255.f,  14/255.f,  15/255.f,
	 16/255.f,  17/255.f,  18/255.f,  19/255.f,  20/255.f,  21/255.f,  22/255.f,  23/255.f,
	 24/255.f,  25/255.f,  26/255.f,  27/255.f,  28/255.f,  29/255.f,  30/255.f,  31/255.f,
	 32/255.f,  33/255.f,  34/255.f,  35/255.f,  36/255.f,  37/255.f,  38/255.f,  39/255.f,
	 40/255.f,  41/255.f,  42/255.f,  43/255.f,  44/255.f,  45/255.f,  46/255.f,  47/255.f,
	 48/255.f,  49/255.f,  50/255.f,  51/255.f,  52/255.f,  53/255.f,  54/255.f,  55/255.f,
	 56/255.f,  57/255.f,  58/255.f,  59/255.f,  60/255.f,  61/255.f,  62/255.f,  63/255.f,
	 64/255.f,  65/255.f,  66/255.f,  67/255.f,  68/255.f,  69/255.f,  70/255.f,  71/255.f,
	 72/255.f,  73/255.f,  74/255.f,  75/255.f,  76/255.f,  77/255.f,  78/255.f,  79/255.f,
	 80/255.f,  81/255.f,  82/255.f,  83/255.f,  84/255.f,  85/255.f,  86/255.f,  87/255.f,
	 88/255.f,  89/255.f,  90/255.f,  91/255.f,  92/255.f,  93/255.f,  94/255.f,  95/255.f,
	 96/255.f,  97/255.f,  98/255.f,  99/255.f, 100/255.f, 101/255.f, 102/255.f, 103/255.f,
	104/255.f, 105/255.f, 106/255.f, 107/255.f, 108/255.f, 109/255.f, 110/255.f, 111/255.f,
	112/255.f, 113/255.f, 114/255.f, 115/255.f, 116/255.f, 117/255.f, 118/255.f, 119/255.f,
	120/255.f, 121/255.f, 122/255.f, 123/255.f, 124/255.f, 125/255.f, 126/255.f, 127/255.f,
	128/255.f, 129/255.f, 130/255.f, 131/255.f, 132/255.f, 133/255.f, 134/255.f, 135/255.f,
	136/255.f, 137/255.f, 138/255.f, 139/255.f, 140/255.f, 141/255.f, 142/255.f, 143/255.f,
	144/255.f, 145/255.f, 146/255.f, 147/255.f, 148/255.f, 149/255.f, 150/255.f, 151/255.f,
	152/255.f, 153/255.f, 154/255.f, 155/255.f, 156/255.f, 157/255.f, 158/255.f, 159/255.f,
	160/255.f, 161/255.f, 162/255.f, 163/255.f, 164/255.f, 165/255.f, 166/255.f, 167/255.f,
	168/255.f, 169/255.f, 170/255.f, 171/255.f, 172/255.f, 173/255.f, 174/255.f, 175/255.f,
	176/255.f, 177/255.f, 178/255.f, 179/255.f, 180/255.f, 181/255.f, 182/255.f, 183/255.f,
	184/255.f, 185/255.f, 186/255.f, 187/255.f, 188/255.f, 189/255.f, 190/255.f, 191/255.f,
	192/255.f, 193/255.f, 194/255.f, 195/255.f, 196/255.f, 197/255.f, 198/255.f, 199/255.f,
	200/255.f, 201/255.f, 202/255.f, 203/255.f, 204/255.f, 205/255.f, 206/255.f, 207/255.f,
	208/255.f, 209/255.f, 210/255.f, 211/255.f, 212/255.f, 213/255.f, 214/255.f, 215/255.f,
	216/255.f, 217/255.f, 218/255.f, 219/255.f, 220/255.f, 221/255.f, 222/255.f, 223/255.f,
	224/255.f, 225/255.f, 226/255.f, 227/255.f, 228/255.f, 229/255.f, 230/255.f, 231/255.f,
	232/255.f, 233/255.f, 234/255.f, 235/255.f, 236/255.f, 237/255.f, 238/255.f, 239/255.f,
	240/255.f, 241/255.f, 242/255.f, 243/255.f, 244/255.f, 245/255.f, 246/255.f, 247/255.f,
	248/255.f, 249/255.f, 250/255.f, 251/255.f, 252/255.f, 253/255.f, 254/255.f, 255/255.f
};

int
libskrift_apply_glyph(LIBSKRIFT_CONTEXT *ctx, const struct libskrift_glyph *glyph, const struct libskrift_colour *colour,
                      int16_t x, int16_t y, struct libskrift_image *image)
{
	struct format_settings settings;
	size_t psize, startr, startc, endr, endc, r, c, n, j;
	size_t img_linesize, gly_linesize, i, gly_psize, usize;
	size_t ri, gi, bi, rj, gj, bj;
	uint16_t x1, y1, x2, y2;
	int16_t sx1, sy1, sx2, sy2;
	uint8_t high;
	uint8_t *img, *img_start = image->image;
	const uint8_t *gly = glyph->image, *gly_start;
	const uint16_t u16 = 0x0102;
	const uint32_t u32 = 0x01020304L;
	const uint64_t u64 = 0x0102030405060708LL;

	if (image->format == LIBSKRIFT_RAW || (unsigned int)image->format > (unsigned int)LEN(libskrift_format_settings)) {
		errno = EINVAL;
		return -1;
	}

	settings = libskrift_format_settings[image->format];
	psize = settings.spsize * (size_t)(4 - (settings.apos <= -2));

	if ((unsigned int)image->endian > LIBSKRIFT_REVERSE_NETWORK_SUBPIXEL ||
	    (settings.float_type && (unsigned int)image->endian < LIBSKRIFT_HOST_SUBPIXEL) ||
	    (!colour && settings.float_type) ||
	    (image->endian % 3 && psize != 1 && psize != 2 && psize != 4 && psize != 8)) {
		errno = EINVAL;
		return -1;
	}

	/* Top left corner of glyph on image */
	sx1 = (int16_t)(x + glyph->x);
	sy1 = (int16_t)(y + glyph->y);

	/* Post-bottom right corner of glyph on image */
	sx2 = (int16_t)(sx1 + (int16_t)glyph->width);
	sy2 = (int16_t)(sy1 + (int16_t)glyph->height);
	if (sx2 <= 0 || sy2 <= 0)
		return 0;

	/* First pixel in image to draw on */
	x1 = sx1 < 0 ? 0 : (uint16_t)sx1;
	y1 = sy1 < 0 ? 0 : (uint16_t)sy1;
	if (x1 >= image->width || y1 >= image->height)
		return 0;

	/* Post-last pixel in image to draw on */
	sx2 = (int16_t)(sx1 + (int16_t)glyph->width);
	sy2 = (int16_t)(sy1 + (int16_t)glyph->height);
	x2 = (uint16_t)sx2 < image->width  ? (uint16_t)sx2 : image->width;
	y2 = (uint16_t)sy2 < image->height ? (uint16_t)sy2 : image->height;

	/* First pixel in glyph to draw */
	startc = (uint16_t)((int16_t)x1 - sx1);
	startr = (uint16_t)((int16_t)y1 - sy1);

	/* Post-last pixel in glyph to draw*/
	endc = (uint16_t)((int16_t)x2 - sx1);
	endr = (uint16_t)((int16_t)y2 - sy1);

	img_linesize = (size_t)image->width * psize;
	img_start += (size_t)y1 * img_linesize;
	img_start += (size_t)x1 * psize;

	gly_psize = (ctx->rendering.smoothing == LIBSKRIFT_SUBPIXEL) ? 3 : 1;
	gly_linesize = (size_t)glyph->width * gly_psize;
	gly += startr * gly_linesize;

	usize = image->endian >= LIBSKRIFT_HOST_SUBPIXEL ? settings.spsize : psize;

#define CHECK_ENDIAN(UPPER, LOWER, BITS)\
	((image->endian == LIBSKRIFT_##UPPER##_PIXEL || image->endian == LIBSKRIFT_##UPPER##_SUBPIXEL) &&\
	 usize == sizeof(uint##BITS##_t) &&\
	 LOWER##BITS##toh(u##BITS) != u##BITS)

#define EACH_UNIT\
		img = img_start, r = startr; r < endr; r++, img += img_linesize)\
			for (n = (endc - startc) * psize, i = 0; i < n; i += usize

	/* Convert endian in image to host endian */
	if      (CHECK_ENDIAN(BE, be, 16)) for (EACH_UNIT) *(uint16_t *)&img[i] = be16toh(*(uint16_t *)&img[i]);
	else if (CHECK_ENDIAN(BE, be, 32)) for (EACH_UNIT) *(uint32_t *)&img[i] = be32toh(*(uint32_t *)&img[i]);
	else if (CHECK_ENDIAN(BE, be, 64)) for (EACH_UNIT) *(uint64_t *)&img[i] = be64toh(*(uint64_t *)&img[i]);
	else if (CHECK_ENDIAN(LE, le, 16)) for (EACH_UNIT) *(uint16_t *)&img[i] = le16toh(*(uint16_t *)&img[i]);
	else if (CHECK_ENDIAN(LE, le, 32)) for (EACH_UNIT) *(uint32_t *)&img[i] = le32toh(*(uint32_t *)&img[i]);
	else if (CHECK_ENDIAN(LE, le, 64)) for (EACH_UNIT) *(uint64_t *)&img[i] = le64toh(*(uint64_t *)&img[i]);

	/* Preprocess (e.g. remove gamma or colour model conversion) */
	if (image->preprocess)
		image->preprocess(image, (size_t)x1, (size_t)y1, (size_t)(x2 - x1), (size_t)(y2 - y1));

	/* Apply glyph */
	if (ctx->rendering.smoothing != LIBSKRIFT_SUBPIXEL)
		ri = 0, gi = 0, bi = 0;
	else if (ctx->subpixel_bgr)
		bi = 0, gi = 1, ri = 2;
	else
		ri = 0, gi = 1, bi = 2;
	startc *= gly_psize;
	endc   *= gly_psize;
	if (colour) {
		switch (settings.float_type) {
		case 0:
			switch (settings.spsize) {
			case 1:
#define TYPE uint8_t
#define NARROW_FLOAT_TYPE float
#define RMAP(Y) uint8_rmap[Y]
#include "apply-glyph.h"
#undef NARROW_FLOAT_TYPE
#undef RMAP
#undef TYPE
				break;
			case 2:
#define TYPE uint16_t
#include "apply-glyph.h"
#undef TYPE
				break;
			case 4:
#define TYPE uint32_t
#include "apply-glyph.h"
#undef TYPE
				break;
			default:
#define TYPE uint64_t
#define WIDE_FLOAT_TYPE long double
#include "apply-glyph.h"
#undef TYPE
#undef WIDE_FLOAT_TYPE
				break;
			}
			break;
		case 1:
#define NARROW_FLOAT_TYPE float
#include "apply-glyph.h"
#undef NARROW_FLOAT_TYPE
			break;
		case 2:
#define NARROW_FLOAT_TYPE double
#include "apply-glyph.h"
#undef NARROW_FLOAT_TYPE
			break;
		default:
#define NARROW_FLOAT_TYPE long double
#include "apply-glyph.h"
#undef NARROW_FLOAT_TYPE
			break;
		}
	} else {
		/* Optimised version for opaque black-on-white/white-on-black, assumes no glyph overlap */
		rj = (size_t)settings.rpos * settings.spsize;
		gj = (size_t)settings.gpos * settings.spsize;
		bj = (size_t)settings.bpos * settings.spsize;
		gly_start = gly;
		for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
			for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
				img[i + rj] ^= gly[c + ri];
				img[i + gj] ^= gly[c + gi];
				img[i + bj] ^= gly[c + bi];
			}
		}
		for (j = 1; j < settings.spsize; j++) {
			ri = rj + j;
			gi = gj + j;
			bi = bj + j;
			gly = gly_start;
			for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
				for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
					img[i + ri] = img[i + rj];
					img[i + gi] = img[i + gj];
					img[i + bi] = img[i + bj];
				}
			}
		}
	}
	startc /= gly_psize;
	endc   /= gly_psize;

	/* Postprocess (e.g. add gamma or colour model conversion) */
	if (image->postprocess)
		image->postprocess(image, (size_t)x1, (size_t)y1, (size_t)(x2 - x1), (size_t)(y2 - y1));

	/* Convert endian in image from host endian */
	if      (CHECK_ENDIAN(BE, be, 16)) for (EACH_UNIT) *(uint16_t *)&img[i] = htobe16(*(uint16_t *)&img[i]);
	else if (CHECK_ENDIAN(BE, be, 32)) for (EACH_UNIT) *(uint32_t *)&img[i] = htobe32(*(uint32_t *)&img[i]);
	else if (CHECK_ENDIAN(BE, be, 64)) for (EACH_UNIT) *(uint64_t *)&img[i] = htobe64(*(uint64_t *)&img[i]);
	else if (CHECK_ENDIAN(LE, le, 16)) for (EACH_UNIT) *(uint16_t *)&img[i] = htole16(*(uint16_t *)&img[i]);
	else if (CHECK_ENDIAN(LE, le, 32)) for (EACH_UNIT) *(uint32_t *)&img[i] = htole32(*(uint32_t *)&img[i]);
	else if (CHECK_ENDIAN(LE, le, 64)) for (EACH_UNIT) *(uint64_t *)&img[i] = htole64(*(uint64_t *)&img[i]);

	return 0;
}
