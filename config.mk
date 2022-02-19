PREFIX    = /usr
MANPREFIX = $(PREFIX)/share/man

DEMO_FONT = /usr/share/fonts/liberation/LiberationSans-Regular.ttf

GUNZIP_PATH = /usr/bin/gunzip
GZIP_SUPPORT = '-DGUNZIP_PATH="$(GUNZIP_PATH)"'
# To exclude gzip support, build with 'GZIP_SUPPORT='

MERGE_STYLE = MAX
# MAX: The max value of the glyph            (minimum legal result, insignificantly slower than SUM)
# OR:  The bitwise OR of the glyph values    (within legal range, fastest)
# SUM: The saturated sum of the glyph values (maximum legal result)

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_GNU_SOURCE \
           -D$(MERGE_STYLE)_MERGE $(GZIP_SUPPORT) '-DDEMO_FONT="$(DEMO_FONT)"'
CFLAGS   = -Wall -g
LDFLAGS  = -lschrift -lm -lgrapheme

CC = c99
