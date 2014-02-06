ACLOCAL_AMFLAGS= -I m4
SUBDIRS = src io_mngr

AM_CFLAGS = -std=gnu99 -lm -DHAVE_CONFIG_H -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2

bin_PROGRAMS = mmforwarder video_receiver video_transmitter video_rec_trans audio_receiver audio_transmitter audio_rec_trans

mmforwarder_SOURCES = tests/mmforwarder.c
mmforwarder_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
mmforwarder_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec -lrtp
mmforwarder_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

video_receiver_SOURCES = tests/video_receiver.c
video_receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
video_receiver_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec -lrtp
video_receiver_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

video_transmitter_SOURCES = tests/video_transmitter.c
video_transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Isrc/rtp -Isrc/utils -Itests -Iio_mngr -Isrc/audio -Idxt_compress
video_transmitter_LDFLAGS = -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec -lrtp
video_transmitter_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

video_rec_trans_SOURCES = tests/video_rec_trans.c
video_rec_trans_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Idxt_compress -Isrc/rtp -Isrc/audio -Isrc/utils
video_rec_trans_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec -lrtp
video_rec_trans_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

audio_receiver_SOURCES = tests/audio_receiver.c
audio_receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
audio_receiver_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec -lrtp
audio_receiver_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

audio_transmitter_SOURCES = tests/audio_transmitter.c
audio_transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
audio_transmitter_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec -lrtp
audio_transmitter_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la

audio_rec_trans_SOURCES = tests/audio_rec_trans.c
audio_rec_trans_CFLAGS = $(AM_CFLAGS) -Isrc -Itests -Iio_mngr -Isrc/rtp -Isrc/utils -Isrc/audio -Idxt_compress
audio_rec_trans_LDFLAGS = -L. -L./src -L./io_mngr -lvcompress -lvdecompress -lacompression -liomanager -lavcodec -lrtp
audio_rec_trans_DEPENDENCIES = src/librtp.la src/libvcompress.la src/libvdecompress.la src/libacompression.la io_mngr/libiomanager.la
