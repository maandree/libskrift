.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OBJ =\
	libskrift_apply_glyph.o\
	libskrift_calculate_dpi.o\
	libskrift_close_font.o\
	libskrift_create_context.o\
	libskrift_draw_text.o\
	libskrift_format_settings.o\
	libskrift_free_context.o\
	libskrift_get_grapheme_glyph.o\
	libskrift_get_cluster_glyph.o\
	libskrift_get_rendering_settings.o\
	libskrift_inches_to_pixels.o\
	libskrift_merge_glyphs.o\
	libskrift_millimeters_to_pixels.o\
	libskrift_open_font.o\
	libskrift_open_font_fd.o\
	libskrift_open_font_file.o\
	libskrift_open_font_mem.o\
	libskrift_points_to_pixels.o\
	libskrift_srgb_postprocess.o\
	libskrift_srgb_preprocess.o

LIB_HDR =\
	libskrift.h

HDR =\
	common.h\
	apply-glyph.h\
	srgb-gamma.h\
	$(LIB_HDR)

all: libskrift.a demo
$(OBJ): $(@:.o=.c) $(HDR)
demo.o: demo.c $(LIB_HDR)

libskrift.a: $(OBJ)
	$(AR) rc $@ $(OBJ)
	$(AR) ts $@ > /dev/null

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

demo: demo.o libskrift.a
	$(CC) -o $@ $@.o libskrift.a $(LDFLAGS)

install: libskrift.a
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	cp -- libskrift.a "$(DESTDIR)$(PREFIX)/lib"
	cp -- libskrift.h "$(DESTDIR)$(PREFIX)/include"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libskrift.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libskrift.h"

clean:
	-rm -f -- *.o *.lo *.su *.a *.so *.so.*

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all install uninstall clean
