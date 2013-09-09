lib_LTLIBRARIES = libiomanager.la

AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
AM_CXXFLAGS = -O2 -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2

dxt_compress:
	cd dxt_compress; make;

libiomanager_la_LDFLAGS = -version-info 0:1:0 -lpthread -L./lib -lrtp -lvcompress -lvdecompress
libiomanager_la_DEPENDENCIES = dxt_compress
libiomanager_la_CFLAGS = $(AM_CFLAGS) -I. -Iio_mngr -Idxt_compress
libiomanager_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Iio_mngr -Idxt_compress
libiomanager_la_SOURCES = ./participants.c \
						  ./receiver.c \
						  ./transmitter.c \
						  ./dxt_compress/dxt_util.c \
						  ./dxt_compress/dxt_decoder.c \
						  ./dxt_compress/dxt_encoder.c

ioincludedir = $(includedir)/ug-modules/io_mngr
ioinclude_HEADERS = participants.h \
				    transmitter.h \
					receiver.h

bin_PROGRAMS = receiver transmitter

receiver_SOURCES = tests/test_receiver.c
receiver_CFLAGS = $(AM_CFLAGS) -I. -Iio_mngr
receiver_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec
receiver_DEPENDENCIES = libiomanager.la

transmitter_SOURCES = tests/test_transmitter.c
transmitter_CFLAGS = $(AM_CFLAGS) -I. -Iio_mngr
transmitter_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec
transmitter_DEPENDENCIES = libiomanager.la
