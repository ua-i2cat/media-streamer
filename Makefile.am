SUBDIRS = dxt_compress src

AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2

bin_PROGRAMS = receiver transmitter

receiver_SOURCES = tests/test_receiver.c
receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Itests
receiver_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec -Lsrc
receiver_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libiomanager.la

transmitter_SOURCES = tests/test_transmitter.c
transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Itests
transmitter_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec -Lsrc
transmitter_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libiomanager.la
