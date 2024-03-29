#include "libskrift.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <alloca.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <grapheme.h>
#include <schrift.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define ELEMSOF(ARR) (sizeof(ARR) / sizeof(*(ARR)))

#define FLAGXOR(F, A, B) ((((F) / (A)) ^ ((F) / (B))) & 1)

#define FLEXSTRUCTSIZE(STRUCT, FLEXARRAY, FLEXARRAY_LENGTH)\
	(offsetof(STRUCT, FLEXARRAY) + (FLEXARRAY_LENGTH) * sizeof(*((STRUCT *)NULL)->FLEXARRAY))

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
	double                     char_x_advancement;
	double                     char_y_advancement;
	double                     text_x_advancement;
	double                     text_y_advancement;
	double                     transformation[6];
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

int libskrift_open_font___(LIBSKRIFT_FONT **fontp, const void *mem_static, void *mem_free, void *mem_unmap, size_t size);
