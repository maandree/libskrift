/* See LICENSE file for copyright and license details. */

static inline long double
gamma_ullf(long double t)
{
	return GAMMA(long double, powl);
}

static inline double
gamma_ulf(double t)
{
	return GAMMA(double, pow);
}

static inline float
gamma_uf(float t)
{
	return GAMMA(float, powf);
}

static inline long double
gamma_llf(long double t)
{
	return t >= 0 ? gamma_ullf(t) : -gamma_ullf(-t);
}

static inline double
gamma_lf(double t)
{
	return t >= 0 ? gamma_ulf(t) : -gamma_ulf(-t);
}

static inline float
gamma_f(float t)
{
	return t >= 0 ? gamma_uf(t) : -gamma_uf(-t);
}

static inline uint64_t
gamma_u64(uint64_t t)
{
	return (uint64_t)(gamma_ullf((long double)t / UINT64_MAX) * UINT64_MAX);
}

static inline uint32_t
gamma_u32(uint32_t t)
{
	return (uint32_t)(gamma_ulf((double)t / UINT32_MAX) * UINT32_MAX);
}

static inline uint16_t
gamma_u16(uint32_t t)
{
	return (uint16_t)(gamma_ulf((double)t / UINT16_MAX) * UINT16_MAX);
}

void
FUNCTION_NAME(struct libskrift_image *image, size_t x, size_t y, size_t width, size_t height)
{
	struct format_settings settings;
	uint8_t *img = image->image;
	size_t linesize, psize;

	settings = libskrift_format_settings[image->format];
	psize    = settings.spsize * (size_t)(4 - (settings.apos <= -2));
	linesize = (size_t)image->width * psize;
	img     += y * linesize + x * psize;
	width   *= psize;

	switch (settings.float_type) {
	case 0:
		switch (settings.spsize) {
		case 1:
			for (y = 0; y < height; y++, img += linesize) {
				for (x = 0; x < width; x += psize) {
					((uint8_t *)&img[x])[settings.rpos] = gamma_u8[((uint8_t *)&img[x])[settings.rpos]];
					((uint8_t *)&img[x])[settings.gpos] = gamma_u8[((uint8_t *)&img[x])[settings.gpos]];
					((uint8_t *)&img[x])[settings.bpos] = gamma_u8[((uint8_t *)&img[x])[settings.bpos]];
				}
			}
			break;

		case 2:
			for (y = 0; y < height; y++, img += linesize) {
				for (x = 0; x < width; x += psize) {
					((uint16_t *)&img[x])[settings.rpos] = gamma_u16(((uint16_t *)&img[x])[settings.rpos]);
					((uint16_t *)&img[x])[settings.gpos] = gamma_u16(((uint16_t *)&img[x])[settings.gpos]);
					((uint16_t *)&img[x])[settings.bpos] = gamma_u16(((uint16_t *)&img[x])[settings.bpos]);
				}
			}
			break;

		case 4:
			for (y = 0; y < height; y++, img += linesize) {
				for (x = 0; x < width; x += psize) {
					((uint32_t *)&img[x])[settings.rpos] = gamma_u32(((uint32_t *)&img[x])[settings.rpos]);
					((uint32_t *)&img[x])[settings.gpos] = gamma_u32(((uint32_t *)&img[x])[settings.gpos]);
					((uint32_t *)&img[x])[settings.bpos] = gamma_u32(((uint32_t *)&img[x])[settings.bpos]);
				}
			}
			break;

		default:
			for (y = 0; y < height; y++, img += linesize) {
				for (x = 0; x < width; x += psize) {
					((uint64_t *)&img[x])[settings.rpos] = gamma_u64(((uint64_t *)&img[x])[settings.rpos]);
					((uint64_t *)&img[x])[settings.gpos] = gamma_u64(((uint64_t *)&img[x])[settings.gpos]);
					((uint64_t *)&img[x])[settings.bpos] = gamma_u64(((uint64_t *)&img[x])[settings.bpos]);
				}
			}
			break;
		}
		break;

	case 1:
		for (y = 0; y < height; y++, img += linesize) {
			for (x = 0; x < width; x += psize) {
				((float *)&img[x])[settings.rpos] = gamma_f(((float *)&img[x])[settings.rpos]);
				((float *)&img[x])[settings.gpos] = gamma_f(((float *)&img[x])[settings.gpos]);
				((float *)&img[x])[settings.bpos] = gamma_f(((float *)&img[x])[settings.bpos]);
			}
		}
		break;

	case 2:
		for (y = 0; y < height; y++, img += linesize) {
			for (x = 0; x < width; x += psize) {
				((double *)&img[x])[settings.rpos] = gamma_lf(((double *)&img[x])[settings.rpos]);
				((double *)&img[x])[settings.gpos] = gamma_lf(((double *)&img[x])[settings.gpos]);
				((double *)&img[x])[settings.bpos] = gamma_lf(((double *)&img[x])[settings.bpos]);
			}
		}
		break;

	default:
		for (y = 0; y < height; y++, img += linesize) {
			for (x = 0; x < width; x += psize) {
				((long double *)&img[x])[settings.rpos] = gamma_llf(((long double *)&img[x])[settings.rpos]);
				((long double *)&img[x])[settings.gpos] = gamma_llf(((long double *)&img[x])[settings.gpos]);
				((long double *)&img[x])[settings.bpos] = gamma_llf(((long double *)&img[x])[settings.bpos]);
			}
		}
		break;
	}
}
