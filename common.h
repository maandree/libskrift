#include "libskrift.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <endian.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <schrift.h>
#include <grapheme.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define LEN(ARR) (sizeof(ARR) / sizeof(*(ARR)))

struct libskrift_font {
	SFT_Font *font;
	void     *memory_free;
	void     *memory_unmap;
	size_t    memory_size;
	size_t    refcount;
};

struct libskrift_context {
	struct libskrift_rendering rendering;
	struct SFT                 schrift_ctx;
	char                       subpixel_horizontally;
	char                       subpixel_vertically;
	char                       subpixel_bgr;
	size_t                     nfonts;
	LIBSKRIFT_FONT            *fonts[];
};

struct format_settings {
	int    float_type;
	int8_t apos;
	int8_t rpos;
	int8_t gpos;
	int8_t bpos;
	size_t spsize;
};

extern const struct format_settings libskrift_format_settings[LIBSKRIFT_RGBA_LONG_DOUBLE + 1];
