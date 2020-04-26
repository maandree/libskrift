/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font(LIBSKRIFT_FONT **fontp, FILE *fp)
{
	int saved_errno;
	char *mem = NULL, *new;
	size_t size = 0, off = 0, r;

	*fontp = calloc(1, sizeof(**fontp));

	saved_errno = errno;
	errno = 0;
	for (;;) {
		if (off + 2048 > size) {
			size += 8192;
			new = realloc(mem, size);
			if (!new) {
				free(mem);
				return -1;
			}
			mem = new;
		}
		r = fread(&mem[off], 1, size - off, fp);
		if (!r)
			break;
		off += r;
	}
	if (errno) {
		free(mem);
		return -1;
	}

	size = off;
	new = realloc(mem, size);
	if (new)
		mem = new;
	errno = saved_errno;

	if (libskrift_open_font_mem(fontp, mem, size)) {
		free(mem);
		return -1;
	}

	(*fontp)->memory_free  = mem;
	(*fontp)->memory_size  = size;

	return 0;
}
