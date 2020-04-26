/* See LICENSE file for copyright and license details. */
#include "common.h"

const struct libskrift_rendering *
libskrift_get_rendering_settings(LIBSKRIFT_CONTEXT *ctx)
{
	return &ctx->rendering;
}
