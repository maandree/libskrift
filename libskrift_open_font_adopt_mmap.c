/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font_adopt_mmap(LIBSKRIFT_FONT **fontp, void *mem, size_t size)
{
	return libskrift_open_font___(fontp, NULL, NULL, mem, size);
}
