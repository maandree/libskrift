/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font_fd(LIBSKRIFT_FONT **fontp, int fd)
{
	int saved_errno;
	struct stat st;
	char *mem = NULL, *new;
	size_t size = 0, off = 0;
	int mmapped = 0;
	ssize_t r;

	saved_errno = errno;
	if (fstat(fd, &st) < 0 || !st.st_size || !S_ISREG(st.st_mode)) {
	fallback:
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
			r = read(fd, &mem[off], size - off);
			if (r <= 0) {
				if (!r)
					break;
				free(mem);
				return -1;
			}
			off += (size_t)r;
		}
		size = off;
		new = realloc(mem, size);
		if (new)
			mem = new;
		errno = saved_errno;
	} else {
		mem = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (mem == MAP_FAILED || !mem)
			goto fallback;
		size = (size_t)st.st_size;
		mmapped = 1;
	}

	if (libskrift_open_font_mem(fontp, mem, size)) {
		if (mmapped)
			munmap(mem, size);
		else
			free(mem);
		return -1;
	}

	(*fontp)->memory_free  = mmapped ? NULL : mem;
	(*fontp)->memory_unmap = mmapped ? mem : NULL;
	(*fontp)->memory_size  = size;

	return 0;
}
