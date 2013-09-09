lib_LTLIBRARIES = librtp.la libvcompress.la libvdecompress.la libiomanager.la


AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
AM_CXXFLAGS = -O2 -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2


librtp_la_LDFLAGS = -version-info 0:1:0 -lrt -ldl -lieee -lm -lcrypto
librtp_la_CFLAGS = $(AM_CFLAGS) -Isrc -Isrc/rtp -Isrc/compat -Isrc/crypto
librtp_la_CXXFLAGS = $(AM_CXXFLAGS) -Isrc -Isrc/rtp -Isrc/compat -Isrc/crypto
librtp_la_SOURCES = src/rtp/net_udp.c \
					src/rtp/pbuf.c \
					src/rtp/ptime.c \
					src/rtp/rtp.c \
					src/rtp/rtp_callback.c \
					src/rtp/rtpdec.c \
					src/crypto/crc_32.c \
					src/crypto/crypt_aes.c \
					src/crypto/crypt_aes_impl.c \
					src/crypto/crypt_des.c \
					src/crypto/md5.c \
					src/crypto/openssl_decrypt.c\
					src/crypto/openssl_encrypt.c \
					src/crypto/random.c \
					src/tv.c \
					src/perf.c \
					src/tfrc.c \
					src/ntp.c \
					src/pdb.c \
					src/debug.c \
					src/compat/gettimeofday.c \
					src/compat/inet_pton.c \
					src/compat/inet_ntop.c \
					src/compat/vsnprintf.c \
					src/compat/drand48.c \
					src/utils/list.c \
					src/transmit.c \
					src/module.c \
					src/messaging.cpp

libvcompress_la_LDFLAGS = -version-info 0:1:0 -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut
libvcompress_la_CFLAGS = $(AM_CFLAGS) -Isrc -Isrc/video_compress -Isrc/utils -Isrc/compat
libvcompress_la_CXXFLAGS = $(AM_CXXFLAGS) -Isrc -Isrc/video_compress -Isrc/utils -Isrc/compat
libvcompress_la_SOURCES = src/video_frame.c \
						  src/gl_context.c \
						  src/glx_common.c \
						  src/video_compress.c \
						  src/debug.c \
						  src/video_codec.c \
						  src/module.c \
						  src/messaging.cpp \
						  src/compat/platform_spin.c \
						  src/compat/platform_semaphore.c \
						  src/x11_common.c \
						  src/video_compress/dxt_glsl.c \
						  src/video_compress/libavcodec.c \
						  src/video_compress/none.c \
						  src/video_compress/uyvy.c \
						  src/utils/list.c \
						  src/utils/resource_manager.cpp \
						  src/utils/worker.cpp

libvdecompress_la_LDFLAGS = -version-info 0:1:0 -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut
libvdecompress_la_CFLAGS = $(AM_CFLAGS) -Isrc -Isrc/video_decompress -Isrc/utils -Isrc/compat
libvdecompress_la_CXXFLAGS = $(AM_CXXFLAGS) -Isrc -Isrc/video_decompress -Isrc/utils -Isrc/compat
libvdecompress_la_SOURCES =  src/video_frame.c \
							 src/gl_context.c \
							 src/glx_common.c \
							 src/video_decompress.c \
							 src/debug.c \
							 src/video_codec.c \
							 src/module.c \
							 src/messaging.cpp \
							 src/compat/platform_spin.c \
							 src/compat/platform_semaphore.c \
							 src/x11_common.c \
							 src/video_decompress/dxt_glsl.c \
							 src/video_decompress/libavcodec.c \
							 src/video_decompress/null.c \
							 src/utils/list.c \
							 src/utils/resource_manager.cpp \
							 src/utils/worker.cpp

libiomanager_la_LDFLAGS = -version-info 0:1:0 -lpthread -L./lib -lrtp -lvcompress -lvdecompress
libiomanager_la_CFLAGS = $(AM_CFLAGS) -Isrc -Iio_mngr -Idxt_compress
libiomanager_la_CXXFLAGS = $(AM_CXXFLAGS) -Isrc -Iio_mngr -Idxt_compress
libiomanager_la_SOURCES = io_mngr/participants.c \
						  io_mngr/receiver.c \
						  io_mngr/transmitter.c \
						  dxt_compress/dxt_util.c \
						  dxt_compress/dxt_decoder.c \
						  dxt_compress/dxt_encoder.c

libiomanager_la_DEPENDENCIES = librtp.la libvcompress.la libvdecompress.la

nobase_include_HEADERS = io_mngr/participants.h \
				         io_mngr/transmitter.h \
						 io_mngr/receiver.h

bin_PROGRAMS = receiver transmitter

receiver_SOURCES = tests/test_receiver.c
receiver_CFLAGS = $(AM_CFLAGS) -Isrc -Iio_mngr
receiver_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec
receiver_DEPENDENCIES = librtp.la libvcompress.la libvdecompress.la libiomanager.la

transmitter_SOURCES = tests/test_transmitter.c
transmitter_CFLAGS = $(AM_CFLAGS) -Isrc -Iio_mngr
transmitter_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec
transmitter_DEPENDENCIES = librtp.la libvcompress.la libvdecompress.la libiomanager.la
