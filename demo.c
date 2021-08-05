/* See LICENSE file for copyright and license details. */
#include "libskrift.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libsimple-arg.h>


#define TEST_TEXT "hello world"


USAGE("[-b background-colour] [-f foreground-colour] [-a opacity] [-t character-transformation] ... "
      "[-T character-transformation] ... [-k kerning] [-i interletter-spacing] [-g grid_fineness] "
      "[-S smoothing-method[:subpixel-order]] [-H hiniting] [-o flags] ... [-s font-size] [-F font-file] "
      "[-D dpi-x:dpi-y | -D dpi] [-w width] [-h height] [-x x-position] [-y y-position] [text]");


static uint8_t
decode_hexbyte(char hi, char lo)
{
	int ret;
	ret  = ((hi & 15) + 9 * (hi > '9')) << 4;
	ret |= ((lo & 15) + 9 * (lo > '9')) << 0;
	return (uint8_t)ret;
}


static void
parse_colour(const char *s, uint8_t *redp, uint8_t *greenp, uint8_t *bluep, uint8_t *alphap)
{
	char rh, rl, gh, gl, bh, bl, ah = 'f', al = 'f';

	if (!isxdigit(s[0])) goto invalid;
	if (!isxdigit(s[1])) goto invalid;
	if (!isxdigit(s[2])) goto invalid;
	if (!isxdigit(s[3])) goto invalid;
	if (!isxdigit(s[4])) goto invalid;
	if (!isxdigit(s[5])) goto invalid;

	rh = *s++;
	rl = *s++;
	gh = *s++;
	gl = *s++;
	bh = *s++;
	bl = *s++;

	if (*s) {
		if (!isxdigit(s[0])) goto invalid;
		if (!isxdigit(s[1])) goto invalid;
		if (s[2])
			goto invalid;
		ah = *s++;
		al = *s++;
	}

	*redp   = decode_hexbyte(rh, rl);
	*greenp = decode_hexbyte(gh, gl);
	*bluep  = decode_hexbyte(bh, bl);
	*alphap = decode_hexbyte(ah, al);
	return;

invalid:
	fprintf(stderr, "%s: colours shall be encoded as rrggbb or rrggbbaa, in hexadecimal\n", argv0);
	exit(1);
}


_LIBSKRIFT_GCC_ONLY(__attribute__((__pure__)))
static uint16_t
parse_uint16(const char *s)
{
	uint16_t ret, digit;
	for (ret = 0; *s; s++) {
		if (!isdigit(*s))
			usage();
		digit = (uint16_t)(*s & 15);
		if (ret > (UINT16_MAX - digit) / 10)
			usage();
		ret *= 10;
		ret += digit;
	}
	return ret;
}


_LIBSKRIFT_GCC_ONLY(__attribute__((__pure__)))
static int16_t
parse_int16(const char *s, const char **end)
{
	int16_t ret, digit;
	int neg = 0;
	if (*s == '+') {
		s++;
	} else if (*s == '-') {
		neg = 1;
		s++;
	}
	for (ret = 0; *s; s++) {
		if (!isdigit(*s))
			break;
		digit = (int16_t)(*s & 15);
		if (ret < (INT16_MIN + digit) / 10)
			usage();
		ret *= 10;
		ret -= digit;
	}
	if (!neg)
		ret = -ret;
	*end = s;
	return ret;
}


static void
parse_transformation(char *s, double matrix[6])
{
#define S(T) T, n = (sizeof(T) - 1)
#define S0(T) (n = (sizeof(T) - 1), T)

	size_t n;
	double f, x, y, m[6];

	while (*s) {
		if (*s == ',') {
			s++;

		} else if (!strncmp(s, S("rotate="))) {
			s += n;
			errno = 0;
			f = strtod(s, &s);
			if (errno)
				goto invalid;
			if (*s) {
				if (s[0] == 'd' && s[1] == 'e' && s[2] == 'g') {
					s += 3;
					libskrift_add_rotation_degrees(matrix, f);
				} else {
					libskrift_add_rotation(matrix, f);
				}
				if (*s && *s != ',')
					goto invalid;
			}

		} else if (!strncmp(s, S("shearx="))) {
			s += n;
			errno = 0;
			f = strtod(s, &s);
			if (errno || (*s && *s != ','))
				goto invalid;
			libskrift_add_shear(matrix, f, 0);

		} else if (!strncmp(s, S("sheary="))) {
			s += n;
			errno = 0;
			f = strtod(s, &s);
			if (errno || (*s && *s != ','))
				goto invalid;
			libskrift_add_shear(matrix, 0, f);

		} else if (!strncmp(s, S("shear="))) {
			s += n;
			errno = 0;
			x = strtod(s, &s);
			if (errno || *s != ':')
				goto invalid;
			y = strtod(&s[1], &s);
			if (errno || (*s && *s != ','))
				goto invalid;
			libskrift_add_shear(matrix, x, y);

		} else if (!strncmp(s, S("scalex="))) {
			s += n;
			errno = 0;
			f = strtod(s, &s);
			if (errno || (*s && *s != ','))
				goto invalid;
			libskrift_add_scaling(matrix, f, 1);

		} else if (!strncmp(s, S("scaley="))) {
			s += n;
			errno = 0;
			f = strtod(s, &s);
			if (errno || (*s && *s != ','))
				goto invalid;
			libskrift_add_scaling(matrix, 1, f);

		} else if (!strncmp(s, S("scale="))) {
			s += n;
			errno = 0;
			x = strtod(s, &s);
			if (errno)
				goto invalid;
			if (*s == ':') {
				y = strtod(&s[1], &s);
				if (errno || (*s && *s != ','))
					goto invalid;
				libskrift_add_scaling(matrix, x, y);
			} else if (*s && *s != ',') {
				goto invalid;
			} else {
				libskrift_add_scaling(matrix, x, x);
			}

		} else if (!strncmp(s, S("translate="))) {
			s += n;
			errno = 0;
			x = strtod(s, &s);
			if (errno || *s != ':')
				goto invalid;
			y = strtod(&s[1], &s);
			if (errno || (*s && *s != ','))
				goto invalid;
			libskrift_add_translation(matrix, x, y);

		} else if (!strncmp(s, S("transform="))) {
			s += n;
			errno = 0;
			m[0] = strtod(s, &s);
			if (errno || *s != ':')
				goto invalid;
			m[1] = strtod(&s[1], &s);
			if (errno || *s != ':')
				goto invalid;
			m[2] = strtod(&s[1], &s);
			if (errno || s[0] != ':' || s[1] != ':')
				goto invalid;
			m[3] = strtod(&s[2], &s);
			if (errno || *s != ':')
				goto invalid;
			m[4] = strtod(&s[1], &s);
			if (errno || *s != ':')
				goto invalid;
			m[5] = strtod(&s[1], &s);
			if (errno || (*s && *s != ','))
				goto invalid;
			libskrift_add_transformation(matrix, m);

		} else if (!strncmp(s, S("rotate90,")) || !strcmp(s, S0("rotate90"))) {
			s += n;
			libskrift_add_90_degree_rotation(matrix);

		} else if (!strncmp(s, S("rotate180,")) || !strcmp(s, S0("rotate180"))) {
			s += n;
			libskrift_add_180_degree_rotation(matrix);

		} else if (!strncmp(s, S("rotate270,")) || !strcmp(s, S0("rotate270"))) {
			s += n;
			libskrift_add_270_degree_rotation(matrix);

		} else if (!strncmp(s, S("transpose,")) || !strcmp(s, S0("transpose"))) {
			s += n;
			libskrift_add_transposition(matrix);

		} else {
		invalid:
			fprintf(stderr, "%s: valid transformations are:\n", argv0);
			fprintf(stderr, "%s:     rotate=<radians>\n", argv0);
			fprintf(stderr, "%s:     rotate=<degrees>deg\n", argv0);
			fprintf(stderr, "%s:     shearx=<value>\n", argv0);
			fprintf(stderr, "%s:     sheary=<value>\n", argv0);
			fprintf(stderr, "%s:     shear=<x-value>:<y-value>\n", argv0);
			fprintf(stderr, "%s:     scalex=<value>\n", argv0);
			fprintf(stderr, "%s:     scaley=<value>\n", argv0);
			fprintf(stderr, "%s:     scale=<value>\n", argv0);
			fprintf(stderr, "%s:     scale=<x-value>:<y-value>\n", argv0);
			fprintf(stderr, "%s:     translate=<x-value>:<y-value>\n", argv0);
			fprintf(stderr, "%s:     transform=<r1c1>:<r1c2>:<r1c3>::<r2c1>:<r2c2>:<r2c3>\n", argv0);
			fprintf(stderr, "%s:     rotate90\n", argv0);
			fprintf(stderr, "%s:     rotate180\n", argv0);
			fprintf(stderr, "%s:     rotate270\n", argv0);
			fprintf(stderr, "%s:     transpose\n", argv0);
			exit(1);
		}
	}

#undef S
#undef S0
}


static double
apply_unit(double size, const char *unit, const struct libskrift_rendering *rendering, const char *unit_for)
{
	if (!strcmp(unit, "px"))
		return size;
	if (!strcmp(unit, "pt"))
		return libskrift_points_to_pixels(size, rendering);
	if (!strcmp(unit, "in"))
		return libskrift_inches_to_pixels(size, rendering);
	if (!strcmp(unit, "mm"))
		return libskrift_millimeters_to_pixels(size, rendering);
	if (!strcmp(unit, "cm"))
		return libskrift_millimeters_to_pixels(size * 10, rendering);

	fprintf(stderr, "%s: valid %s units are: 'px', 'pt', 'in', 'mm', 'cm'", argv0, unit_for);
	exit(1);
}


int
main(int argc, char *argv[])
{
	LIBSKRIFT_FONT *font;
	LIBSKRIFT_CONTEXT *ctx;
	struct libskrift_image image = {LIBSKRIFT_R8G8B8A8, LIBSKRIFT_BE_SUBPIXEL, 0, 800, 600, NULL, NULL, NULL};
	struct libskrift_rendering rendering = LIBSKRIFT_DEFAULT_RENDERING;
	struct libskrift_colour colour;
	const char *font_file = DEMO_FONT;
	double font_size = 72;
	const char *font_size_unit = "pt";
	const char *x_unit = "", *y_unit = "";
	const char *kerning_unit = "", *interletter_spacing_unit = "";
	double height, opacity = .80f, f;
	size_t size, i;
	char *end, *arg;
	const char *text = TEST_TEXT;
	size_t text_length = sizeof(TEST_TEXT) - 1;
	uint8_t background_red   = 32U,  foreground_red   = 204U;
	uint8_t background_green = 48U,  foreground_green = 128U;
	uint8_t background_blue  = 64U,  foreground_blue  = 51U;
	uint8_t background_alpha = 250U, foreground_alpha = 128U;
	long int tmp_long;
	int16_t x = 0, y = 300;

	ARGBEGIN {
	case 'b':
		parse_colour(ARG(), &background_red, &background_green, &background_blue, &background_alpha);
		break;

	case 'f':
		parse_colour(ARG(), &foreground_red, &foreground_green, &foreground_blue, &foreground_alpha);
		break;

	case 'F':
		font_file = ARG();
		break;

	case 'a':
		errno = 0;
		opacity = strtod(ARG(), &end);
		if (errno || *end)
			usage();
		break;

	case 's':
		errno = 0;
		font_size = strtod(ARG(), &end);
		if (errno)
			usage();
		font_size_unit = end;
		break;

	case 'S':
		errno = 0;
		arg = ARG();
		if (!strncmp(arg, "monochrome", strlen("monochrome"))) {
			arg += strlen("monochrome");
			rendering.smoothing = LIBSKRIFT_MONOCHROME;
		} else if (!strncmp(arg, "none", strlen("none"))) {
			arg += strlen("none");
			rendering.smoothing = LIBSKRIFT_NONE; /* = LIBSKRIFT_MONOCHROME */
		} else if (!strncmp(arg, "greyscale", strlen("greyscale"))) {
			arg += strlen("greyscale");
			rendering.smoothing = LIBSKRIFT_GREYSCALE;
		} else if (!strncmp(arg, "subpixel", strlen("subpixel"))) {
			arg += strlen("subpixel");
			rendering.smoothing = LIBSKRIFT_SUBPIXEL;
		} else {
			fprintf(stderr, "%s: valid smoothing methods are: 'monochrome', 'greyscale', 'subpixel'\n", argv0);
			return 1;
		}
		if (!*arg) {
			rendering.subpixel_order = LIBSKRIFT_RGB;
			break;
		}
		if (*arg++ != ':')
			usage();
		if (!strcmp(arg, "other")) {
			rendering.subpixel_order = LIBSKRIFT_OTHER;
		} else if (!strcmp(arg, "none")) {
			rendering.subpixel_order = LIBSKRIFT_NONE; /* = LIBSKRIFT_OTHER */
		} else if (!strcmp(arg, "rgb")) {
			rendering.subpixel_order = LIBSKRIFT_RGB;
		} else if (!strcmp(arg, "bgr")) {
			rendering.subpixel_order = LIBSKRIFT_BGR;
		} else if (!strcmp(arg, "vrgb")) {
			rendering.subpixel_order = LIBSKRIFT_VRGB;
		} else if (!strcmp(arg, "vbgr")) {
			rendering.subpixel_order = LIBSKRIFT_VBGR;
		} else {
			fprintf(stderr, "%s: valid subpixel orders are: 'other', 'rgb', 'bgr', 'vrgb', 'cbgr'\n", argv0);
			return 1;
		}
		break;

	case 'H':
		arg = ARG();
		if (!strcmp(arg, "none")) {
			rendering.hinting = LIBSKRIFT_NONE;
		} else if (!strcmp(arg, "unhinted")) {
			rendering.hinting = LIBSKRIFT_UNHINTED; /* = LIBSKRIFT_NONE */
		} else if (!strcmp(arg, "slight")) {
			rendering.hinting = LIBSKRIFT_SLIGHT;
		} else if (!strcmp(arg, "medium")) {
			rendering.hinting = LIBSKRIFT_MEDIUM;
		} else if (!strcmp(arg, "full")) {
			rendering.hinting = LIBSKRIFT_FULL;
		} else if (isdigit(*arg)) {
			rendering.hinting = 0;
			for (; isdigit(*arg); arg++) {
				rendering.hinting *= 10;
				rendering.hinting += *arg & 15;
			}
			if (*arg)
				usage();
		} else {
			fprintf(stderr, "%s: valid hintings are: 'none', 'slight', 'medium', 'full', <integer>\n", argv0);
			return 1;
		}
		break;

	case 'D':
		errno = 0;
		rendering.horizontal_dpi = strtod(ARG(), &end);
		if (errno)
			usage();
		if (!*end) {
			rendering.vertical_dpi = rendering.horizontal_dpi;
		} else if (*end != ':') {
			usage();
		} else {
			rendering.vertical_dpi = strtod(&end[1], &end);
			if (errno || *end)
				usage();
		}
		break;

	case 'w':
		image.width = parse_uint16(ARG());
		break;

	case 'h':
		image.height = parse_uint16(ARG());
		break;

	case 'x':
		x = parse_int16(ARG(), &x_unit);
		break;

	case 'y':
		y = parse_int16(ARG(), &y_unit);
		break;

	case 't':
		parse_transformation(ARG(), rendering.char_transformation);
		break;

	case 'T':
		parse_transformation(ARG(), rendering.text_transformation);
		break;

	case 'k':
		errno = 0;
		rendering.kerning = strtod(ARG(), &end);
		if (errno)
			usage();
		kerning_unit = end;
		break;

	case 'i':
		errno = 0;
		rendering.interletter_spacing = strtod(ARG(), &end);
		if (errno)
			usage();
		interletter_spacing_unit = end;
		break;

	case 'g':
		errno = 0;
		tmp_long = strtol(ARG(), &end, 10);
		rendering.grid_fineness = (int)tmp_long;
		if (errno || *end || tmp_long < INT_MIN || tmp_long > INT_MAX)
			usage();
		break;

	case 'o':
		arg = ARG();
		while (*arg) {
			if (*arg == ',') {
				arg++;
				continue;
			}
#define TEST(T) strncmp(arg, T, sizeof(T) - 1) ? 0 : (arg += sizeof(T) - 1, 1)
			if (TEST("remove-gamma")) {
				rendering.flags |= LIBSKRIFT_REMOVE_GAMMA;
			} else if (TEST("y-increases-upwards")) {
				rendering.flags |= LIBSKRIFT_Y_INCREASES_UPWARDS;
			} else if (TEST("flip-text")) {
				rendering.flags ^= LIBSKRIFT_FLIP_TEXT;
			} else if (TEST("flip-chars")) {
				rendering.flags ^= LIBSKRIFT_FLIP_CHARS;
			} else if (TEST("mirror-text")) {
				rendering.flags ^= LIBSKRIFT_MIRROR_TEXT;
			} else if (TEST("mirror-chars")) {
				rendering.flags ^= LIBSKRIFT_MIRROR_CHARS;
			} else if (TEST("transpose-text")) {
				rendering.flags ^= LIBSKRIFT_TRANSPOSE_TEXT;
			} else if (TEST("transpose-chars")) {
				rendering.flags ^= LIBSKRIFT_TRANSPOSE_CHARS;
			} else if (TEST("no-ligatures")) {
				rendering.flags |= LIBSKRIFT_NO_LIGATURES;
			} else if (TEST("advance-char-to-grid")) {
				rendering.flags |= LIBSKRIFT_ADVANCE_CHAR_TO_GRID;
			} else if (TEST("regress-char-to-grid")) {
				rendering.flags |= LIBSKRIFT_REGRESS_CHAR_TO_GRID;
			} else if (TEST("advance-word-to-grid")) {
				rendering.flags |= LIBSKRIFT_ADVANCE_WORD_TO_GRID;
			} else if (TEST("regress-word-to-grid")) {
				rendering.flags |= LIBSKRIFT_REGRESS_WORD_TO_GRID;
			} else if (TEST("use-subpixel-grid")) {
				rendering.flags |= LIBSKRIFT_USE_SUBPIXEL_GRID;
			} else if (TEST("vertical-text")) {
				rendering.flags |= LIBSKRIFT_VERTICAL_TEXT;
			} else if (TEST("autohinting")) {
				rendering.flags |= LIBSKRIFT_AUTOHINTING;
			} else if (TEST("no-autohinting")) {
				rendering.flags |= LIBSKRIFT_NO_AUTOHINTING;
			} else if (TEST("autokerning")) {
				rendering.flags |= LIBSKRIFT_AUTOKERNING;
			} else if (TEST("no-autokerning")) {
				rendering.flags |= LIBSKRIFT_NO_AUTOKERNING;
			} else if (TEST("ignore-ignorable")) {
				rendering.flags |= LIBSKRIFT_IGNORE_IGNORABLE;
			} else {
			invalid_flag:
				fprintf(stderr, "%s: valid flags are:\n", argv0);
				fprintf(stderr, "%s:     remove-gamma\n", argv0);
				fprintf(stderr, "%s:     y-increases-upwards\n", argv0);
				fprintf(stderr, "%s:     flip-text\n", argv0);
				fprintf(stderr, "%s:     flip-chars\n", argv0);
				fprintf(stderr, "%s:     mirror-text\n", argv0);
				fprintf(stderr, "%s:     mirror-chars\n", argv0);
				fprintf(stderr, "%s:     transpose-text\n", argv0);
				fprintf(stderr, "%s:     transpose-chars\n", argv0);
				fprintf(stderr, "%s:     no-ligatures\n", argv0);
				fprintf(stderr, "%s:     advance-char-to-grid\n", argv0);
				fprintf(stderr, "%s:     regress-char-to-grid\n", argv0);
				fprintf(stderr, "%s:     advance-word-to-grid\n", argv0);
				fprintf(stderr, "%s:     regress-word-to-grid\n", argv0);
				fprintf(stderr, "%s:     use-subpixel-grid\n", argv0);
				fprintf(stderr, "%s:     vertical-text\n", argv0);
				fprintf(stderr, "%s:     autohinting\n", argv0);
				fprintf(stderr, "%s:     no-autohinting\n", argv0);
				fprintf(stderr, "%s:     autokerning\n", argv0);
				fprintf(stderr, "%s:     no-autokerning\n", argv0);
				fprintf(stderr, "%s:     ignore-ignorable\n", argv0);
				return 1;
			}
#undef TEST
			if (*arg && *arg != ',')
				goto invalid_flag;
		}
		break;

	default:
		usage();
	} ARGEND;

	colour = (struct libskrift_colour)LIBSKRIFT_PREMULTIPLY((float)opacity,
	                                                        (float)foreground_alpha / 255,
	                                                        (float)foreground_red   / 255,
	                                                        (float)foreground_green / 255,
	                                                        (float)foreground_blue  / 255);

	if (argc > 1)
		usage();

	if (argc == 1) {
		text = argv[0];
		text_length = strlen(text);
	}

	if (isatty(STDOUT_FILENO)) {
		fprintf(stderr, "Output file is a binary PAM image file, I am not writing that to a terminal.\n");
		return 1;
	}

	if (*x_unit && strcmp(x_unit, "px")) {
		f = (double)x;
		f = apply_unit(f, x_unit, &rendering, "position");
		x = (int16_t)f;
	}

	if (*y_unit && strcmp(y_unit, "px")) {
		f = (double)y;
		f = apply_unit(f, y_unit, &rendering, "position");
		y = (int16_t)f;
	}

	if (*kerning_unit && strcmp(kerning_unit, "px")) {
		f = rendering.kerning;
		f = apply_unit(f, kerning_unit, &rendering, "spacing");
		rendering.kerning = f;
	}

	if (*interletter_spacing_unit && strcmp(interletter_spacing_unit, "px")) {
		f = rendering.interletter_spacing;
		f = apply_unit(f, interletter_spacing_unit, &rendering, "spacing");
		rendering.interletter_spacing = f;
	}

	if (libskrift_open_font_file(&font, font_file)) {
		perror("libskrift_open_font_file");
		return 1;
	}
	height = apply_unit(font_size, font_size_unit, &rendering, "font size");
	if (libskrift_create_context(&ctx, &font, 1, height, &rendering, NULL)) {
		perror("libskrift_create_context");
		return 1;
	}
	libskrift_close_font(font);

	size = 4;
	size *= (size_t)image.width;
	size *= (size_t)image.height;
	if (size) {
		image.image = malloc(size);
		if (!image.image) {
			perror("malloc");
			return 1;
		}
	}
	for (i = 0; i < size; i += 4) {
		((uint8_t *)image.image)[i + 0] = background_red;
		((uint8_t *)image.image)[i + 1] = background_green;
		((uint8_t *)image.image)[i + 2] = background_blue;
		((uint8_t *)image.image)[i + 3] = background_alpha;
	}

	if (libskrift_draw_text(ctx, text, text_length, &colour, x, y, &image) < 0) {
		perror("libskrift_draw_text");
		return 1;
	}

	printf("P7\n");
	printf("WIDTH %u\n", image.width);
	printf("HEIGHT %u\n", image.height);
	printf("DEPTH 4\n");
	printf("MAXVAL 255\n");
	printf("TUPLTYPE RGB_ALPHA\n");
	printf("ENDHDR\n");
	fwrite(image.image, 1, size, stdout);
	fflush(stdout);

	free(image.image);
	libskrift_free_context(ctx);
	return 0;
}
