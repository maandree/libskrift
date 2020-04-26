#include "libskrift.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <schrift.h>
#include <grapheme.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

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
