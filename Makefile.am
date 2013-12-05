ACLOCAL_AMFLAGS= -I m4
SUBDIRS = src io_mngr

AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2

bin_PROGRAMS = receiver transmitter rec_trans audio_receiver audio_transmitter

receiver_SOURCES = tests/test_receiver.c
receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
receiver_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec
receiver_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

transmitter_SOURCES = tests/test_transmitter.c
transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Isrc/rtp -Isrc/utils -Itests -Iio_mngr -Isrc/audio -Idxt_compress
transmitter_LDFLAGS = -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec
transmitter_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

rec_trans_SOURCES = tests/test_rec_trans.c
rec_trans_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Idxt_compress -Isrc/rtp -Isrc/audio -Isrc/utils
rec_trans_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec
rec_trans_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

audio_receiver_SOURCES = tests/audio_receiver.c
audio_receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
audio_receiver_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec
audio_receiver_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

audio_transmitter_SOURCES = tests/audio_transmitter.c
audio_transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
audio_transmitter_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec
audio_transmitter_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la
