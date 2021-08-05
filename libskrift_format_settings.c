/* See LICENSE file for copyright and license details. */
#include "common.h"

#define  RGBA  3, 0, 1, 2
#define  RGB  -2, 0, 1, 2
#define XRGB  -1, 1, 2, 3
#define ARGB   0, 1, 2, 3

const struct format_settings libskrift_format_settings[] = {
	[LIBSKRIFT_R8G8B8]           = {0,  RGB,  sizeof(uint8_t)},
	[LIBSKRIFT_X8R8G8B8]         = {0, XRGB,  sizeof(uint8_t)},
	[LIBSKRIFT_A8R8G8B8]         = {0, ARGB,  sizeof(uint8_t)},
	[LIBSKRIFT_R8G8B8A8]         = {0,  RGBA, sizeof(uint8_t)},
	[LIBSKRIFT_R16G16B16]        = {0,  RGB,  sizeof(uint16_t)},
	[LIBSKRIFT_X16R16G16B16]     = {0, XRGB,  sizeof(uint16_t)},
	[LIBSKRIFT_A16R16G16B16]     = {0, ARGB,  sizeof(uint16_t)},
	[LIBSKRIFT_R16G16B16A16]     = {0,  RGBA, sizeof(uint16_t)},
	[LIBSKRIFT_R32G32B32]        = {0,  RGB,  sizeof(uint32_t)},
	[LIBSKRIFT_X32R32G32B32]     = {0, XRGB,  sizeof(uint32_t)},
	[LIBSKRIFT_A32R32G32B32]     = {0, ARGB,  sizeof(uint32_t)},
	[LIBSKRIFT_R32G32B32A32]     = {0,  RGBA, sizeof(uint32_t)},
	[LIBSKRIFT_R64G64B64]        = {0,  RGB,  sizeof(uint64_t)},
	[LIBSKRIFT_X64R64G64B64]     = {0, XRGB,  sizeof(uint64_t)},
	[LIBSKRIFT_A64R64G64B64]     = {0, ARGB,  sizeof(uint64_t)},
	[LIBSKRIFT_R64G64B64A64]     = {0,  RGBA, sizeof(uint64_t)},
	[LIBSKRIFT_RGB_FLOAT]        = {1,  RGB,  sizeof(float)},
	[LIBSKRIFT_ARGB_FLOAT]       = {1, ARGB,  sizeof(float)},
	[LIBSKRIFT_RGBA_FLOAT]       = {1,  RGBA, sizeof(float)},
	[LIBSKRIFT_RGB_DOUBLE]       = {2,  RGB,  sizeof(double)},
	[LIBSKRIFT_ARGB_DOUBLE]      = {2, ARGB,  sizeof(double)},
	[LIBSKRIFT_RGBA_DOUBLE]      = {2,  RGBA, sizeof(double)},
	[LIBSKRIFT_RGB_LONG_DOUBLE]  = {3,  RGB,  sizeof(long double)},
	[LIBSKRIFT_ARGB_LONG_DOUBLE] = {3, ARGB,  sizeof(long double)},
	[LIBSKRIFT_RGBA_LONG_DOUBLE] = {3,  RGBA, sizeof(long double)}
};
