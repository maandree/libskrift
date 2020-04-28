/* See LICENSE file for copyright and license details. */
#include "common.h"

#define FORCED_FLAGS (LIBSKRIFT_NO_LIGATURES   |\
                      LIBSKRIFT_NO_AUTOHINTING |\
                      LIBSKRIFT_NO_AUTOKERNING)

#define IMPLEMENTED_FLAGS (LIBSKRIFT_REMOVE_GAMMA |\
                           FORCED_FLAGS) /* libschrift does not add gamma, so not handling is required */

#define COPY_ARRAY(DEST_STRUCT, SRC_STRUCT, FIELD)\
	memcpy((DEST_STRUCT).FIELD, (SRC_STRUCT).FIELD, sizeof((SRC_STRUCT).FIELD))

static const struct libskrift_rendering default_rendering = LIBSKRIFT_DEFAULT_RENDERING;

static int
transformation_hook(void *hook_data, double advance, double transform[6])
{
	LIBSKRIFT_CONTEXT *ctx = hook_data;
	double r1c1b = ctx->rendering.char_transformation[0], r1c1a = transform[0];
	double r1c2b = ctx->rendering.char_transformation[1], r1c2a = transform[2];
	double r1c3b = ctx->rendering.char_transformation[2], r1c3a = transform[4];
	double r2c1b = ctx->rendering.char_transformation[3], r2c1a = transform[1];
	double r2c2b = ctx->rendering.char_transformation[4], r2c2a = transform[3];
	double r2c3b = ctx->rendering.char_transformation[5], r2c3a = transform[5];
	transform[0] = r1c1a * r1c1b + r1c2a * r2c1b;
	transform[2] = r1c1a * r1c2b + r1c2a * r2c2b;
	transform[4] = r1c1a * r1c3b + r1c2a * r2c3b + r1c3a;
	transform[1] = r2c1a * r1c1b + r2c2a * r2c1b;
	transform[3] = r2c1a * r1c2b + r2c2a * r2c2b;
	transform[5] = r2c1a * r1c3b + r2c2a * r2c3b + r2c3a;
	(void) advance;
	return 0;
}

int
libskrift_create_context(LIBSKRIFT_CONTEXT **ctxp, LIBSKRIFT_FONT **fonts, size_t nfonts, double height,
                         const struct libskrift_rendering *rendering, void *caching)
{
	size_t i;

	(void) caching;

	if (!nfonts) {
		errno = EINVAL;
		return -1;
	}

	if (rendering) {
		if (!rendering->grid_fineness) {
			errno = EINVAL;
			return -1;
		}
	}

	*ctxp = calloc(1, offsetof(LIBSKRIFT_CONTEXT, fonts) + nfonts * sizeof(*(*ctxp)->fonts));
	if (!*ctxp)
		return -1;

	(*ctxp)->schrift_ctx.font   = fonts[0]->font;
	(*ctxp)->schrift_ctx.yScale = height;
	(*ctxp)->nfonts             = nfonts;
	for (i = 0; i < nfonts; i++) {
		(*ctxp)->fonts[i] = fonts[i];
		fonts[i]->refcount += 1;
	}

	if (rendering) {
		memcpy(&(*ctxp)->rendering, rendering, sizeof(*rendering));
		COPY_ARRAY((*ctxp)->rendering, default_rendering, prestroke_transformation_rotation);
		COPY_ARRAY((*ctxp)->rendering, default_rendering, left_transformation);
		COPY_ARRAY((*ctxp)->rendering, default_rendering, right_transformation);
		COPY_ARRAY((*ctxp)->rendering, default_rendering, top_transformation);
		COPY_ARRAY((*ctxp)->rendering, default_rendering, bottom_transformation);
		COPY_ARRAY((*ctxp)->rendering, default_rendering, poststroke_transformation_rotation);
		COPY_ARRAY((*ctxp)->rendering, default_rendering, text_transformation);
	} else {
		memcpy(&(*ctxp)->rendering, &default_rendering, sizeof(default_rendering));
	}

	(*ctxp)->rendering.struct_version      = LIBSKRIFT_RENDERING_STRUCT_VERSION;
	(*ctxp)->rendering.hinting             = LIBSKRIFT_NONE;
	(*ctxp)->rendering.flags              &= IMPLEMENTED_FLAGS;
	(*ctxp)->rendering.flags              |= FORCED_FLAGS;
	(*ctxp)->rendering.grid_fineness       = 1;
	(*ctxp)->rendering.kerning             = 0;
	(*ctxp)->rendering.interletter_spacing = 0;

	if (!(*ctxp)->rendering.smoothing)
		(*ctxp)->rendering.smoothing = LIBSKRIFT_GREYSCALE;

	(*ctxp)->schrift_ctx.xScale = (*ctxp)->schrift_ctx.yScale;
	(*ctxp)->schrift_ctx.xScale *= (*ctxp)->rendering.horizontal_dpi / (*ctxp)->rendering.vertical_dpi;
	(*ctxp)->schrift_ctx.hook_data = (*ctxp);
	if (fpclassify((*ctxp)->rendering.char_transformation[0] - 1) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[1]) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[2]) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[3]) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[4] - 1) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[5]) != FP_ZERO)
		(*ctxp)->schrift_ctx.transformation_hook = transformation_hook;

	if ((*ctxp)->rendering.smoothing == LIBSKRIFT_SUBPIXEL) {
		if ((*ctxp)->rendering.subpixel_order == LIBSKRIFT_RGB) {
			(*ctxp)->schrift_ctx.xScale   *= 3;
			(*ctxp)->subpixel_horizontally = 1;
		} else if ((*ctxp)->rendering.subpixel_order == LIBSKRIFT_BGR) {
			(*ctxp)->schrift_ctx.xScale   *= 3;
			(*ctxp)->subpixel_horizontally = 1;
			(*ctxp)->subpixel_bgr          = 1;
		} else if ((*ctxp)->rendering.subpixel_order == LIBSKRIFT_VRGB) {
			(*ctxp)->schrift_ctx.yScale   *= 3;
			(*ctxp)->subpixel_vertically   = 1;
		} else if ((*ctxp)->rendering.subpixel_order == LIBSKRIFT_VBGR) {
			(*ctxp)->schrift_ctx.yScale   *= 3;
			(*ctxp)->subpixel_vertically   = 1;
			(*ctxp)->subpixel_bgr          = 1;
		} else {
			(*ctxp)->rendering.subpixel_order = LIBSKRIFT_NONE;
			(*ctxp)->rendering.smoothing = LIBSKRIFT_GREYSCALE;
		}
	}

	return 0;
}
