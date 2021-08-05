/* See LICENSE file for copyright and license details. */
#include "common.h"

#define FORCED_FLAGS (LIBSKRIFT_NO_LIGATURES   |\
                      LIBSKRIFT_NO_AUTOHINTING |\
                      LIBSKRIFT_NO_AUTOKERNING)

#define IMPLEMENTED_FLAGS (LIBSKRIFT_REMOVE_GAMMA |\
                           LIBSKRIFT_MIRROR_TEXT  |\
                           LIBSKRIFT_MIRROR_CHARS |\
                           FORCED_FLAGS) /* libschrift does not add gamma, so not handling is required */

#define TRANSFORMING_FLAGS (LIBSKRIFT_MIRROR_TEXT    |\
                            LIBSKRIFT_MIRROR_CHARS   |\
                            LIBSKRIFT_TRANSPOSE_TEXT |\
                            LIBSKRIFT_TRANSPOSE_CHARS)

#define COPY_ARRAY(DEST_STRUCT, SRC_STRUCT, FIELD)\
	memcpy((DEST_STRUCT).FIELD, (SRC_STRUCT).FIELD, sizeof((SRC_STRUCT).FIELD))

static const struct libskrift_rendering default_rendering = LIBSKRIFT_DEFAULT_RENDERING;

static void
multiply_matrices(const double a[restrict 6], const double b[restrict 6], double r[restrict 6])
{
	r[0] = a[0] * b[0] + a[1] * b[3];
	r[1] = a[0] * b[1] + a[1] * b[4];
	r[2] = a[0] * b[2] + a[1] * b[5] + a[2];
	r[3] = a[3] * b[0] + a[4] * b[3];
	r[4] = a[3] * b[1] + a[4] * b[4];
	r[5] = a[3] * b[2] + a[4] * b[5] + a[5];
}

static int
transformation_hook(void *hook_data, double advance, double transform[6])
{
	LIBSKRIFT_CONTEXT *ctx = hook_data;
	double schrift[6] = {transform[0], transform[2], transform[4],
	                     transform[1], transform[3], transform[5]};
	double m1[6] = {1, 0, 0, 0, 1, 0}, m2[6] = {1, 0, 0, 0, 1, 0};
	if (((ctx->rendering.flags / LIBSKRIFT_MIRROR_CHARS) ^ (ctx->rendering.flags / LIBSKRIFT_MIRROR_TEXT)) & 1)
		multiply_matrices((double []){-1, 0, advance / schrift[0], 0, 1, 0}, m2, m1);
	multiply_matrices(ctx->transformation, m1, m2);
	multiply_matrices(schrift, m2, m1);
	transform[0] = m1[0];
	transform[2] = m1[1];
	transform[4] = m1[2];
	transform[1] = m1[3];
	transform[3] = m1[4];
	transform[5] = m1[5];
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

	*ctxp = calloc(1, FLEXSTRUCTSIZE(LIBSKRIFT_CONTEXT, fonts, nfonts));
	if (!*ctxp)
		return -1;

	(*ctxp)->schrift_ctx.font   = fonts[0]->font;
	(*ctxp)->schrift_ctx.yScale = height;
	(*ctxp)->char_x_advancement = 1;
	(*ctxp)->char_y_advancement = 0;
	(*ctxp)->text_x_advancement = 1;
	(*ctxp)->text_y_advancement = 0;
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
	} else {
		memcpy(&(*ctxp)->rendering, &default_rendering, sizeof(default_rendering));
	}

	(*ctxp)->rendering.struct_version = LIBSKRIFT_RENDERING_STRUCT_VERSION;
	(*ctxp)->rendering.hinting        = LIBSKRIFT_NONE;
	(*ctxp)->rendering.flags         &= IMPLEMENTED_FLAGS;
	(*ctxp)->rendering.flags         |= FORCED_FLAGS;
	(*ctxp)->rendering.flags         |= LIBSKRIFT_REMOVE_GAMMA; /* libschrift does not add gamma */
	(*ctxp)->rendering.grid_fineness  = 1;
	(*ctxp)->rendering.kerning        = 0;

	if (!(*ctxp)->rendering.smoothing)
		(*ctxp)->rendering.smoothing = LIBSKRIFT_GREYSCALE;

	(*ctxp)->schrift_ctx.xScale = (*ctxp)->schrift_ctx.yScale;
	(*ctxp)->schrift_ctx.xScale *= (*ctxp)->rendering.horizontal_dpi / (*ctxp)->rendering.vertical_dpi;
	(*ctxp)->schrift_ctx.hook_data = (*ctxp);
	if (((*ctxp)->rendering.flags & TRANSFORMING_FLAGS) ||
	    fpclassify((*ctxp)->rendering.char_transformation[0] - 1) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[1])     != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[2])     != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[3])     != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[4] - 1) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.char_transformation[5])     != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.text_transformation[0] - 1) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.text_transformation[1])     != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.text_transformation[2])     != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.text_transformation[3])     != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.text_transformation[4] - 1) != FP_ZERO ||
	    fpclassify((*ctxp)->rendering.text_transformation[5])     != FP_ZERO) {
		memcpy((*ctxp)->transformation, (*ctxp)->rendering.char_transformation, sizeof((*ctxp)->transformation));
		libskrift_add_transformation((*ctxp)->transformation, (*ctxp)->rendering.text_transformation);
		(*ctxp)->char_x_advancement = (*ctxp)->rendering.char_transformation[0];
		(*ctxp)->char_y_advancement = (*ctxp)->rendering.char_transformation[3];
		(*ctxp)->text_x_advancement = (*ctxp)->rendering.text_transformation[0];
		(*ctxp)->text_y_advancement = (*ctxp)->rendering.text_transformation[3];
		(*ctxp)->schrift_ctx.transformation_hook = transformation_hook;
	}

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
