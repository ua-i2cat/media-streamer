lib_LTLIBRARIES = librtp.la libvcompress.la libvdecompress.la libiomanager.la


AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
AM_CXXFLAGS = -O2 -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2


librtp_la_LDFLAGS = -version-info 0:1:0 -lrt -ldl -lieee -lm -lcrypto
librtp_la_CFLAGS = $(AM_CFLAGS) -I. -Irtp -Icompat -Icrypto
librtp_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Irtp -Icompat -Icrypto
librtp_la_SOURCES = rtp/net_udp.c \
					rtp/pbuf.c \
					rtp/ptime.c \
					rtp/rtp.c \
					rtp/rtp_callback.c \
					rtp/rtpdec.c \
					crypto/crc_32.c \
					crypto/crypt_aes.c \
					crypto/crypt_aes_impl.c \
					crypto/crypt_des.c \
					crypto/md5.c \
					crypto/openssl_decrypt.c\
					crypto/openssl_encrypt.c \
					crypto/random.c \
					tv.c \
					perf.c \
					tfrc.c \
					ntp.c \
					pdb.c \
					debug.c \
					compat/gettimeofday.c \
					compat/inet_pton.c \
					compat/inet_ntop.c \
					compat/vsnprintf.c \
					compat/drand48.c \
					utils/list.c \
					transmit.c \
					module.c \
					messaging.cpp

libvcompress_la_LDFLAGS = -version-info 0:1:0 -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut
libvcompress_la_CFLAGS = $(AM_CFLAGS) -I. -Ivideo_compress -Iutils -Icompat
libvcompress_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Ivideo_compress -Iutils -Icompat
libvcompress_la_SOURCES = video_frame.c \
						  gl_context.c \
						  glx_common.c \
						  video_compress.c \
						  debug.c \
						  video_codec.c \
						  module.c \
						  messaging.cpp \
						  compat/platform_spin.c \
						  compat/platform_semaphore.c \
						  x11_common.c \
						  video_compress/dxt_glsl.c \
						  video_compress/libavcodec.c \
						  video_compress/none.c \
						  video_compress/uyvy.c \
						  utils/list.c \
						  utils/resource_manager.cpp \
						  utils/worker.cpp

libvdecompress_la_LDFLAGS = -version-info 0:1:0 -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut
libvdecompress_la_CFLAGS = $(AM_CFLAGS) -I. -Ivideo_decompress -Iutils -Icompat
libvdecompress_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Ivideo_decompress -Iutils -Icompat
libvdecompress_la_SOURCES =  video_frame.c \
							 gl_context.c \
							 glx_common.c \
							 video_decompress.c \
							 debug.c \
							 video_codec.c \
							 module.c \
							 messaging.cpp \
							 compat/platform_spin.c \
							 compat/platform_semaphore.c \
							 x11_common.c \
							 video_decompress/dxt_glsl.c \
							 video_decompress/libavcodec.c \
							 video_decompress/null.c \
							 utils/list.c \
							 utils/resource_manager.cpp \
							 utils/worker.cpp

libiomanager_la_LDFLAGS = -version-info 0:1:0 -lpthread -L./lib -lrtp -lvcompress -lvdecompress
libiomanager_la_CFLAGS = $(AM_CFLAGS) -I. -Iio_mngr -Idxt_compress
libiomanager_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Iio_mngr -Idxt_compress
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
receiver_CFLAGS = $(AM_CFLAGS) -I. -Iio_mngr
receiver_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec
receiver_DEPENDENCIES = librtp.la libvcompress.la libvdecompress.la libiomanager.la

transmitter_SOURCES = tests/test_transmitter.c
transmitter_CFLAGS = $(AM_CFLAGS) -I. -Iio_mngr
transmitter_LDFLAGS = -lvcompress -lvdecompress -liomanager -lavcodec
transmitter_DEPENDENCIES = librtp.la libvcompress.la libvdecompress.la libiomanager.la
