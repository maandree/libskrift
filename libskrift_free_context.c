/* See LICENSE file for copyright and license details. */
#include "common.h"

void
libskrift_free_context(LIBSKRIFT_CONTEXT *ctx)
{
	if (ctx) {
		libskrift_close_font(ctx->font);
		free(ctx);
	}
}
