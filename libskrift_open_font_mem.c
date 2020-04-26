/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font_mem(LIBSKRIFT_FONT **fontp, const void *mem, size_t size)
{
	*fontp = calloc(1, sizeof(**fontp));
	if (!*fontp)
		return -1;
	(*fontp)->refcount = 1;
	(*fontp)->font = sft_loadmem(mem, size);
	if (!(*fontp)->font) {
		free(*fontp);
		*fontp = NULL;
		return -1;
	}
	return 0;
}
