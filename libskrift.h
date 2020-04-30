/* See LICENSE file for copyright and license details. */
#ifndef LIBSKRIFT_H
#define LIBSKRIFT_H 1

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__GNUC__)
# define _LIBSKRIFT_GCC_ONLY(...) __VA_ARGS__
#else
# define _LIBSKRIFT_GCC_ONLY(...)
#endif


#define LIBSKRIFT_RENDERING_STRUCT_VERSION 0

#define LIBSKRIFT_NONE 0

typedef struct libskrift_context LIBSKRIFT_CONTEXT;
typedef struct libskrift_font LIBSKRIFT_FONT;
typedef uint_least32_t libskrift_codepoint_t;


enum libskrift_format {
	LIBSKRIFT_RAW,
	LIBSKRIFT_R8G8B8,
	LIBSKRIFT_X8R8G8B8,
	LIBSKRIFT_A8R8G8B8,
	LIBSKRIFT_R8G8B8A8,
	LIBSKRIFT_R16G16B16,
	LIBSKRIFT_X16R16G16B16,
	LIBSKRIFT_A16R16G16B16,
	LIBSKRIFT_R16G16B16A16,
	LIBSKRIFT_R32G32B32,
	LIBSKRIFT_X32R32G32B32,
	LIBSKRIFT_A32R32G32B32,
	LIBSKRIFT_R32G32B32A32,
	LIBSKRIFT_R64G64B64,
	LIBSKRIFT_X64R64G64B64,
	LIBSKRIFT_A64R64G64B64,
	LIBSKRIFT_R64G64B64A64,
	LIBSKRIFT_RGB_FLOAT,
	LIBSKRIFT_ARGB_FLOAT,
	LIBSKRIFT_RGBA_FLOAT,
	LIBSKRIFT_RGB_DOUBLE,
	LIBSKRIFT_ARGB_DOUBLE,
	LIBSKRIFT_RGBA_DOUBLE,
	LIBSKRIFT_RGB_LONG_DOUBLE,
	LIBSKRIFT_ARGB_LONG_DOUBLE,
	LIBSKRIFT_RGBA_LONG_DOUBLE
};

enum libskrift_endian {
	LIBSKRIFT_HOST_PIXEL,
	LIBSKRIFT_NETWORK_PIXEL,
	LIBSKRIFT_REVERSE_NETWORK_PIXEL,
	LIBSKRIFT_HOST_SUBPIXEL,
	LIBSKRIFT_NETWORK_SUBPIXEL,
	LIBSKRIFT_REVERSE_NETWORK_SUBPIXEL

#define LIBSKRIFT_BE_PIXEL    LIBSKRIFT_NETWORK_PIXEL
#define LIBSKRIFT_BE_SUBPIXEL LIBSKRIFT_NETWORK_SUBPIXEL
#define LIBSKRIFT_LE_PIXEL    LIBSKRIFT_REVERSE_NETWORK_PIXEL
#define LIBSKRIFT_LE_SUBPIXEL LIBSKRIFT_REVERSE_NETWORK_SUBPIXEL
};

enum libskrift_subpixel_order {
	LIBSKRIFT_OTHER, /* LIBSKRIFT_NONE */
	LIBSKRIFT_RGB,
	LIBSKRIFT_BGR,
	LIBSKRIFT_VRGB,
	LIBSKRIFT_VBGR
};

enum libskrift_smoothing {
	LIBSKRIFT_MONOCHROME, /* LIBSKRIFT_NONE */
	LIBSKRIFT_GREYSCALE,
	LIBSKRIFT_SUBPIXEL
};

enum libskrift_hinting {
	LIBSKRIFT_UNHINTED = 0, /* LIBSKRIFT_NONE */
	LIBSKRIFT_SLIGHT   = 25,
	LIBSKRIFT_MEDIUM   = 50,
	LIBSKRIFT_FULL     = 100
};

#define LIBSKRIFT_REMOVE_GAMMA         UINT32_C(0x00000001)
#define LIBSKRIFT_Y_INCREASES_UPWARDS  UINT32_C(0x00000002) /* SFT_DOWNWARD_Y otherwise */
#define LIBSKRIFT_FLIP_TEXT            UINT32_C(0x00000004)
#define LIBSKRIFT_FLIP_CHARS           UINT32_C(0x00000008)
#define LIBSKRIFT_MIRROR_TEXT          UINT32_C(0x00000010)
#define LIBSKRIFT_MIRROR_CHARS         UINT32_C(0x00000020)
#define LIBSKRIFT_TRANSPOSE_TEXT       UINT32_C(0x00000040)
#define LIBSKRIFT_TRANSPOSE_CHARS      UINT32_C(0x00000080)
#define LIBSKRIFT_NO_LIGATURES         UINT32_C(0x00000100)
#define LIBSKRIFT_ADVANCE_CHAR_TO_GRID UINT32_C(0x00000200)
#define LIBSKRIFT_REGRESS_CHAR_TO_GRID UINT32_C(0x00000400) /* Combine with LIBSKRIFT_ADVANCE_CHAR_TO_GRID for closest alternative */
#define LIBSKRIFT_ADVANCE_WORD_TO_GRID UINT32_C(0x00000800)
#define LIBSKRIFT_REGRESS_WORD_TO_GRID UINT32_C(0x00001000) /* Combine with LIBSKRIFT_ADVANCE_WORD_TO_GRID for closest alternative */
#define LIBSKRIFT_USE_SUBPIXEL_GRID    UINT32_C(0x00002000)
#define LIBSKRIFT_VERTICAL_TEXT        UINT32_C(0x00004000)
#define LIBSKRIFT_AUTOHINTING          UINT32_C(0x00008000) /* Use autohinter even if hint information exists */
#define LIBSKRIFT_NO_AUTOHINTING       UINT32_C(0x00010000) /* Use autohinter if no hint information exist */
#define LIBSKRIFT_AUTOKERNING          UINT32_C(0x00020000) /* Use autokerner even if kerning information exists */
#define LIBSKRIFT_NO_AUTOKERNING       UINT32_C(0x00040000) /* Use autokerner if no kerning information exist */

struct libskrift_rendering {
	int struct_version;
	enum libskrift_subpixel_order subpixel_order;
	enum libskrift_smoothing      smoothing;
	enum libskrift_hinting        hinting;
	uint32_t flags;
	int      grid_fineness;
	double   horizontal_dpi;
	double   vertical_dpi;
	double   kerning;
	double   interletter_spacing;
	double   prestroke_transformation_rotation[4];
	double   left_transformation[6];
	double   right_transformation[6];
	double   top_transformation[6];
	double   bottom_transformation[6];
	double   poststroke_transformation_rotation[4];
	double   char_transformation[6];
	double   text_transformation[6];
};

struct libskrift_glyph {
	double   advance;
	int16_t  x;
	int16_t  y;
	uint16_t width;
	uint16_t height;
	size_t   size;
	uint8_t  image[];
};

struct libskrift_saved_grapheme {
	libskrift_codepoint_t cp;
	size_t len;
};

struct libskrift_image {
	enum libskrift_format format;
	enum libskrift_endian endian;
	int                   premultiplied;
	uint16_t              width;
	uint16_t              height;
	void                (*preprocess)(struct libskrift_image *image, size_t x, size_t y, size_t width, size_t height);
	void                (*postprocess)(struct libskrift_image *image, size_t x, size_t y, size_t width, size_t height);
	void                 *image;
};

struct libskrift_colour {
	double opacity; /* [0, 1] */
	double alpha;   /* [0, .opacity] */
	double red;     /* [0, .alpha] */
	double green;   /* [0, .alpha] */
	double blue;    /* [0, .alpha] */
};


#define LIBSKRIFT_DEFAULT_RENDERING {\
	.struct_version                     = LIBSKRIFT_RENDERING_STRUCT_VERSION,\
	.subpixel_order                     = LIBSKRIFT_NONE,\
	.smoothing                          = LIBSKRIFT_GREYSCALE,\
	.hinting                            = LIBSKRIFT_FULL,\
	.flags                              = 0,\
	.grid_fineness                      = 1,\
	.horizontal_dpi                     = (double)1920 * 254 / 5180,\
	.vertical_dpi                       = (double)1200 * 254 / 3240,\
	.kerning                            = 1,\
	.interletter_spacing                = 0,\
	.prestroke_transformation_rotation  = {1, 0,      0, 1},\
	.left_transformation                = {1, 0, 0,   0, 1, 0},\
	.right_transformation               = {1, 0, 0,   0, 1, 0},\
	.top_transformation                 = {1, 0, 0,   0, 1, 0},\
	.bottom_transformation              = {1, 0, 0,   0, 1, 0},\
	.poststroke_transformation_rotation = {1, 0,      0, 1},\
	.char_transformation                = {1, 0, 0,   0, 1, 0},\
	.text_transformation                = {1, 0, 0,   0, 1, 0},\
}

#define LIBSKRIFT_NO_SAVED_GRAPHEME {0, 0}

#define LIBSKRIFT_PREMULTIPLY(OPACITY, ALPHA, RED, GREEN, BLUE)\
	{(OPACITY),\
	 (ALPHA) * (OPACITY),\
	 (RED)   * (ALPHA) * (OPACITY),\
	 (GREEN) * (ALPHA) * (OPACITY),\
	 (BLUE)  * (ALPHA) * (OPACITY)}


_LIBSKRIFT_GCC_ONLY(__attribute__((__const__, __warn_unused_result__)))
inline double
libskrift_calculate_dpi(double pixels, double millimeters)
{
	return pixels * 254 / 10 / millimeters;
}

_LIBSKRIFT_GCC_ONLY(__attribute__((__const__, __warn_unused_result__)))
inline double
libskrift_inches_to_pixels(double inches, const struct libskrift_rendering *rendering)
{
	return inches * rendering->vertical_dpi;
}

_LIBSKRIFT_GCC_ONLY(__attribute__((__const__, __warn_unused_result__)))
inline double
libskrift_millimeters_to_pixels(double millimeters, const struct libskrift_rendering *rendering)
{
	return millimeters * 10 / 254 * rendering->vertical_dpi;
}

_LIBSKRIFT_GCC_ONLY(__attribute__((__const__, __warn_unused_result__)))
inline double
libskrift_points_to_pixels(double points, const struct libskrift_rendering *rendering)
{
	return points / 72 * rendering->vertical_dpi;
}


_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
int libskrift_open_font_file(LIBSKRIFT_FONT **, const char *);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
int libskrift_open_font_mem(LIBSKRIFT_FONT **, const void *, size_t);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
int libskrift_open_font_fd(LIBSKRIFT_FONT **, int);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
int libskrift_open_font_at(LIBSKRIFT_FONT **, int, const char *);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
int libskrift_open_font(LIBSKRIFT_FONT **, FILE *);

void libskrift_close_font(LIBSKRIFT_FONT *);


_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__(1, 2))))
int libskrift_create_context(LIBSKRIFT_CONTEXT **, LIBSKRIFT_FONT **, size_t, double, const struct libskrift_rendering *, void *);

void libskrift_free_context(LIBSKRIFT_CONTEXT *);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__, __returns_nonnull__, __warn_unused_result__, __const__)))
const struct libskrift_rendering *libskrift_get_rendering_settings(LIBSKRIFT_CONTEXT *);


_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
int libskrift_get_grapheme_glyph(LIBSKRIFT_CONTEXT *, libskrift_codepoint_t, double, double, struct libskrift_glyph **);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__(1, 2, 6))))
ssize_t libskrift_get_cluster_glyph(LIBSKRIFT_CONTEXT *, const char *, struct libskrift_saved_grapheme *,
                                    double, double, struct libskrift_glyph **);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
int libskrift_merge_glyphs(LIBSKRIFT_CONTEXT *, const struct libskrift_glyph *, const struct libskrift_glyph *,
                           struct libskrift_glyph **);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__(1, 2, 6))))
int libskrift_apply_glyph(LIBSKRIFT_CONTEXT *, const struct libskrift_glyph *, const struct libskrift_colour *,
                          int16_t, int16_t, struct libskrift_image *);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__(1, 2, 6))))
int libskrift_draw_text(LIBSKRIFT_CONTEXT *, const char *, const struct libskrift_colour *,
                        int16_t, int16_t, struct libskrift_image *);


_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
void libskrift_srgb_preprocess(struct libskrift_image *, size_t, size_t, size_t, size_t);

_LIBSKRIFT_GCC_ONLY(__attribute__((__nonnull__)))
void libskrift_srgb_postprocess(struct libskrift_image *, size_t, size_t, size_t, size_t);


inline void
libskrift_add_transformation(double m[restrict 6], const double tm[restrict 6])
{
	double a = m[0], b = m[1], c = m[2];
	double d = m[3], e = m[4], f = m[5];
	m[0] = tm[0] * a + tm[1] * d;
	m[1] = tm[0] * b + tm[1] * e;
	m[2] = tm[0] * c + tm[1] * f + tm[2];
	m[3] = tm[3] * a + tm[4] * d;
	m[4] = tm[3] * b + tm[4] * e;
	m[5] = tm[3] * c + tm[4] * f + tm[5];
}

inline void
libskrift_add_rotation(double m[6], double radians)
{
	double c = cos(-radians), s = sin(-radians);
	libskrift_add_transformation(m, (double []){c, -s, 0, s, c, 0});
}

inline void
libskrift_add_rotation_degrees(double m[6], double degrees)
{
	libskrift_add_rotation(m, degrees * (double)0.017453292519943295f);
}

inline void
libskrift_add_90_degree_rotation(double m[6])
{
	double a = m[0], b = m[1], c = m[2];
	m[0] = m[3], m[1] = m[4], m[2] = m[5];
	m[3] = -a,   m[4] = -b,   m[5] = -c;
}

inline void
libskrift_add_180_degree_rotation(double m[6])
{
	m[0] = -m[0], m[1] = -m[1], m[2] = -m[2];
	m[3] = -m[3], m[4] = -m[4], m[5] = -m[5];
}

inline void
libskrift_add_270_degree_rotation(double m[6])
{
	double a = m[0], b = m[1], c = m[2];
	m[0] = -m[3], m[1] = -m[4], m[2] = -m[5];
	m[3] = a,     m[4] = b,     m[5] = c;
}

inline void
libskrift_add_scaling(double m[6], double x, double y)
{
	m[0] *= x, m[1] *= x, m[2] *= x;
	m[3] *= y, m[4] *= y, m[5] *= y;
}

inline void
libskrift_add_transposition(double m[6])
{
	double a = m[0], b = m[1], c = m[2];
	m[0] = m[3], m[1] = m[4], m[2] = m[5];
	m[3] = a,    m[4] = b,    m[5] = c;
}

inline void
libskrift_add_shear(double m[6], double x, double y)
{
	double a = m[0], b = m[1], c = m[2];
	double d = m[3], e = m[4], f = m[6];
	m[0] += x * d, m[1] += x * e, m[2] += x * f;
	m[3] += y * a, m[4] += y * b, m[5] += y * c;
}

inline void
libskrift_add_translation(double m[6], double x, double y)
{
	m[2] += x;
	m[5] += y;
}

#endif
