/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font_mem(LIBSKRIFT_FONT **fontp, const void *mem, size_t size)
{
	return libskrift_open_font___(fontp, mem, NULL, NULL, size);
}
