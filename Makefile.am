SUBDIRS = src io_mngr

AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2

bin_PROGRAMS = receiver transmitter


receiver_SOURCES = tests/test_receiver.c
receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Idxt_compress
receiver_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -liomanager -lavcodec
receiver_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la io_mngr/libiomanager.la

transmitter_SOURCES = tests/test_transmitter.c
transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Isrc/rtp -Isrc/utils -Itests -Iio_mngr -Idxt_compress
transmitter_LDFLAGS = -L./src -L./io_mngr -lvcompress -lvdecompress -liomanager -lavcodec
transmitter_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la io_mngr/libiomanager.la
