/* See LICENSE file for copyright and license details. */
#include "common.h"

void
libskrift_close_font(LIBSKRIFT_FONT *font)
{
	if (font && !--font->refcount) {
		sft_freefont(font->font);
		free(font->memory_free);
		if (font->memory_unmap)
			munmap(font->memory_unmap, font->memory_size);
		free(font);
	}
}
