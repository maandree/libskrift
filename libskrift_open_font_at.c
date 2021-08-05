/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font_at(LIBSKRIFT_FONT **fontp, int dirfd, const char *path)
{
	int fd, ret, saved_errno;
	if (!*path) {
		return libskrift_open_font_fd(fontp, dirfd);
	} else {
		fd = openat(dirfd, path, O_RDONLY);
		if (fd < 0)
			return -1;
		ret = libskrift_open_font_fd(fontp, fd);
		saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return ret;
	}
}
