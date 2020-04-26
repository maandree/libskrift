PREFIX    = /usr
MANPREFIX = $(PREFIX)/share/man

DEMO_FONT = /usr/share/fonts/liberation/LiberationSans-Regular.ttf

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_GNU_SOURCE '-DDEMO_FONT="$(DEMO_FONT)"'
CFLAGS   = -std=c99 -Wall -g
LDFLAGS  = -lschrift -lm -lgrapheme

CC = cc
