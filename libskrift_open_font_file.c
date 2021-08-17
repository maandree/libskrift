/* See LICENSE file for copyright and license details. */
#include "common.h"

int
libskrift_open_font_file(LIBSKRIFT_FONT **fontp, const char *path)
{
	if (!*path) {
		errno = EINVAL;
		return -1;
	}
	return libskrift_open_font_at(fontp, AT_FDCWD, path);
}
