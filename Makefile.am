SUBDIRS = src io_mngr

AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2

bin_PROGRAMS = receiver transmitter #rec_trans audio_rtp_only


receiver_SOURCES = tests/test_receiver.c
receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
receiver_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -liomanager -lavcodec
receiver_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la io_mngr/libiomanager.la

transmitter_SOURCES = tests/test_transmitter.c
transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Isrc/rtp -Isrc/utils -Itests -Iio_mngr -Isrc/audio -Idxt_compress
transmitter_LDFLAGS = -L./src -L./io_mngr -lvcompress -lvdecompress -liomanager -lavcodec
transmitter_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la io_mngr/libiomanager.la

rec_trans_SOURCES = tests/test_rec_trans.c
rec_trans_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Idxt_compress -Isrc/rtp -Isrc/audio -Isrc/utils
rec_trans_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -liomanager -lavcodec
rec_trans_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la io_mngr/libiomanager.la

audio_rtp_only_SOURCES = tests/audio_rtp_only.c
audio_rtp_only_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Isrc/rtp -Isrc/utils -Isrc/audio
audio_rtp_only_LDFLAGS = -L./src -lrtp
audio_rtp_only_DEPENDENCIES = src/librtp.la
