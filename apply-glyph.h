/* See LICENSE file for copyright and license details. */

#ifndef NARROW_FLOAT_TYPE
# define NARROW_FLOAT_TYPE double
# define NARROW_FLOAT_TYPE_AUTO
#endif
#ifndef WIDE_FLOAT_TYPE
# define WIDE_FLOAT_TYPE NARROW_FLOAT_TYPE
# define WIDE_FLOAT_TYPE_AUTO
#endif
#ifndef TYPE
# define TYPE NARROW_FLOAT_TYPE
# define TYPE_AUTO
# define TYPEMAX 1
#else
# define TYPEMAX ((TYPE)~(TYPE)0)
#endif
#ifndef RMAP
# define RMAP(Y) ((NARROW_FLOAT_TYPE)((WIDE_FLOAT_TYPE)(Y) / TYPEMAX))
# define RMAP_AUTO
#endif
#define MAP(Y) (TYPE)((WIDE_FLOAT_TYPE)(Y) * TYPEMAX + (WIDE_FLOAT_TYPE)0.5f)
#define R      (&((TYPE *)&img[i])[settings.rpos])
#define G      (&((TYPE *)&img[i])[settings.gpos])
#define B      (&((TYPE *)&img[i])[settings.bpos])
#define A      (&((TYPE *)&img[i])[settings.apos])

do {
	NARROW_FLOAT_TYPE opacity   = (NARROW_FLOAT_TYPE)colour->opacity / 255;
	NARROW_FLOAT_TYPE al, alpha = (NARROW_FLOAT_TYPE)colour->alpha   / 255;
	NARROW_FLOAT_TYPE re, red   = (NARROW_FLOAT_TYPE)colour->red     / 255;
	NARROW_FLOAT_TYPE gr, green = (NARROW_FLOAT_TYPE)colour->green   / 255;
	NARROW_FLOAT_TYPE bl, blue  = (NARROW_FLOAT_TYPE)colour->blue    / 255;
	NARROW_FLOAT_TYPE transparency;

	if (settings.apos < 0) {
		if (gly_psize > 1) {
			for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
				for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
					*R = MAP(RMAP(*R) * (1 - opacity * gly[c + ri]) + gly[c + ri] * red);
					*G = MAP(RMAP(*G) * (1 - opacity * gly[c + gi]) + gly[c + gi] * green);
					*B = MAP(RMAP(*B) * (1 - opacity * gly[c + bi]) + gly[c + bi] * blue);
				}
			}
		} else {
			for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
				for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
					transparency = 1 - opacity * gly[c + ri];
					*R = MAP(RMAP(*R) * transparency + gly[c + ri] * red);
					*G = MAP(RMAP(*G) * transparency + gly[c + gi] * green);
					*B = MAP(RMAP(*B) * transparency + gly[c + bi] * blue);
				}
			}
		}
	} else if (image->premultiplied) {
		if (gly_psize > 1) {
			for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
				for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
					high = gly[c + ri] > gly[c + gi] ? gly[c + ri] : gly[c + gi];
					high = high        > gly[c + bi] ? high        : gly[c + bi];
					*R = MAP(RMAP(*R) * (1 - opacity * gly[c + ri]) + gly[c + ri] * red);
					*G = MAP(RMAP(*G) * (1 - opacity * gly[c + gi]) + gly[c + gi] * green);
					*B = MAP(RMAP(*B) * (1 - opacity * gly[c + bi]) + gly[c + bi] * blue);
					*A = MAP(RMAP(*A) * (1 - opacity * high)        + high        * alpha);
				}
			}
		} else {
			for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
				for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
					transparency = 1 - opacity * gly[c + ri];
					*R = MAP(RMAP(*R) * transparency + gly[c + ri] * red);
					*G = MAP(RMAP(*G) * transparency + gly[c + ri] * green);
					*B = MAP(RMAP(*B) * transparency + gly[c + ri] * blue);
					*A = MAP(RMAP(*A) * transparency + gly[c + ri] * alpha);
				}
			}
		}
	} else {
		if (gly_psize > 1) {
			for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
				for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
					high = gly[c + ri] > gly[c + gi] ? gly[c + ri] : gly[c + gi];
					high = high        > gly[c + bi] ? high        : gly[c + bi];
					re = RMAP(*R) * RMAP(*A) * (1 - opacity * gly[c + ri]) + gly[c + ri] * red;
					gr = RMAP(*G) * RMAP(*A) * (1 - opacity * gly[c + gi]) + gly[c + gi] * green;
					bl = RMAP(*B) * RMAP(*A) * (1 - opacity * gly[c + bi]) + gly[c + bi] * blue;
					al = RMAP(*A) * (1 - opacity * high) + high * alpha;
					if (fpclassify(al) != FP_ZERO) {
						*R = MAP(re);
						*G = MAP(gr);
						*B = MAP(bl);
						*A = MAP(al);
					} else {
						*R = *G = *B = *A = 0;
					}
				}
			}
		} else {
			for (img = img_start, r = startr; r < endr; r++, img += img_linesize, gly += gly_linesize) {
				for (c = startc, i = 0; c < endc; c += gly_psize, i += psize) {
					transparency = 1 - opacity * gly[c + ri];
					re = RMAP(*R) * RMAP(*A) * transparency + gly[c + ri] * red;
					gr = RMAP(*G) * RMAP(*A) * transparency + gly[c + ri] * green;
					bl = RMAP(*B) * RMAP(*A) * transparency + gly[c + ri] * blue;
					al = RMAP(*A) * transparency + gly[c + ri] * alpha;
					if (fpclassify(al) != FP_ZERO) {
						*R = MAP(re);
						*G = MAP(gr);
						*B = MAP(bl);
						*A = MAP(al);
					} else {
						*R = *G = *B = *A = 0;
					}
				}
			}
		}
	}
} while (0);

#ifdef NARROW_FLOAT_TYPE_AUTO
# undef NARROW_FLOAT_TYPE_AUTO
# undef NARROW_FLOAT_TYPE
#endif
#ifdef WIDE_FLOAT_TYPE_AUTO
# undef WIDE_FLOAT_TYPE_AUTO
# undef WIDE_FLOAT_TYPE
#endif
#ifdef TYPE_AUTO
# undef TYPE_AUTO
# undef TYPE
#endif
#undef TYPEMAX
#ifdef RMAP_AUTO
# undef RMAP_AUTO
# undef RMAP
#endif
#undef MAP
#undef R
#undef G
#undef B
#undef A
