/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font_adopt_mem(LIBSKRIFT_FONT **fontp, void *mem, size_t size)
{
	return libskrift_open_font___(fontp, NULL, mem, NULL, size);
}
