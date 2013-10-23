lib_LTLIBRARIES = librtp.la libvcompress.la libvdecompress.la


AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
AM_CXXFLAGS = -O2 -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2


librtp_la_LDFLAGS = -version-info 0:1:0 -lrt -ldl -lieee -lm -lcrypto
librtp_la_CFLAGS = $(AM_CFLAGS) -I. -Irtp -Iutils -Icompat -Icrypto
librtp_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Irtp -Iutils -Icompat -Icrypto
librtp_la_SOURCES = rtp/net_udp.c \
					rtp/pbuf.c \
					rtp/ptime.c \
					rtp/rtp.c \
					rtp/rtp_callback.c \
					rtp/rtpdec.c \
					rtp/audio_decoders.c \
					crypto/crc_32.c \
					crypto/crypt_aes.c \
					crypto/crypt_aes_impl.c \
					crypto/crypt_des.c \
					crypto/md5.c \
					crypto/openssl_decrypt.c\
					crypto/openssl_encrypt.c \
					crypto/random.c \
                                        audio/utils.c \
                                        audio/codec.c \
					tv.c \
					perf.c \
					tfrc.c \
					ntp.c \
					pdb.c \
					debug.c \
					transmit.c \
					module.c \
					messaging.cpp \
					video_codec.c \
					video_frame.c \
					compat/platform_spin.c \
					compat/gettimeofday.c \
					compat/inet_pton.c \
					compat/inet_ntop.c \
					compat/vsnprintf.c \
					compat/drand48.c \
					utils/list.c \
					utils/h264_stream.c

libvcompress_la_LDFLAGS = -version-info 0:1:0 -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut -lGLU
libvcompress_la_CFLAGS = $(AM_CFLAGS) -I. -Ivideo_compress -Iutils -Icompat -I.. -I../dxt_compress
libvcompress_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Ivideo_compress -Iutils -Icompat -I.. -I../dxt_compress
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
#						  video_compress/fastdxt.c \
#						  ../dxt_compress/dxt_common.c \
#						  ../dxt_compress/dxt_decoder.c \
#						  ../dxt_compress/dxt_display.c \
#						  ../dxt_compress/dxt_encoder.c \
#						  ../dxt_compress/dxt_util.c

libvdecompress_la_LDFLAGS = -version-info 0:1:0 -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut -lGLU
libvdecompress_la_CFLAGS = $(AM_CFLAGS) -I. -Ivideo_decompress -Iutils -Icompat -I.. -I../dxt_compress
libvdecompress_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -Ivideo_decompress -Iutils -Icompat -I.. -I../dxt_compress
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
							 utils/packet_counter.cpp \
							 utils/worker.cpp
#							 ../dxt_compress/dxt_common.c \
#							 ../dxt_compress/dxt_decoder.c \
#							 ../dxt_compress/dxt_display.c \
#							 ../dxt_compress/dxt_encoder.c \
#							 ../dxt_compress/dxt_util.c

ugincludedir = $(includedir)/ug-modules
nobase_uginclude_HEADERS =	./tfrc.h \
							./utils/worker.h \
							./utils/resource_manager.h \
							./utils/lock_guard.h \
							./utils/list.h \
							./utils/h264_stream.h \
							./utils/bs.h \
							./utils/packet_counter.h \
							./ntp.h \
							./config.h \
							./perf.h \
							./video_codec.h \
							./config_win32.h \
							./tv.h \
							./video_decompress/libavcodec.h \
							./video_decompress/jpeg.h \
							./video_decompress/null.h \
							./video_decompress/dxt_glsl.h \
							./video_decompress/jpeg_to_dxt.h \
							./messaging.h \
							./transmit.h \
							./types.h \
							./compat/inet_ntop.h \
							./compat/platform_semaphore.h \
							./compat/gettimeofday.h \
							./compat/vsnprintf.h \
							./compat/platform_spin.h \
							./compat/inet_pton.h \
							./compat/drand48.h \
							./config_unix.h \
							./video_compress/libavcodec.h \
							./video_compress/none.h \
							./video_compress/uyvy.h \
							./video_compress/jpeg.h \
							./video_compress/dxt_glsl.h \
							./video_compress/fastdxt.h \
							./video_compress.h \
							./crypto/random.h \
							./crypto/crypt_des.h \
							./crypto/crypt_aes_impl.h \
							./crypto/md5.h \
							./crypto/openssl_encrypt.h \
							./crypto/crc.h \
							./crypto/crypt_aes.h \
							./crypto/openssl_decrypt.h \
							./glx_common.h \
							./libavcodec_common.h \
							./video_frame.h \
							./gl_context.h \
							./pdb.h \
							./video.h \
							./module.h \
							./x11_common.h \
							./lib_common.h \
							./debug.h \
							./rtp/pbuf.h \
							./rtp/rtp_callback.h \
							./rtp/net_udp.h \
							./rtp/rtpdec.h \
							./rtp/ptime.h \
							./rtp/rtp.h \
							./rtp/audio_decoders.h \
							./video_decompress.h \
                                                        ./audio/audio.h \
                                                        ./audio/utils.h \
                                                        ./audio/codec.h \
  						    ../dxt_compress/dxt_common.h \
						    ../dxt_compress/dxt_decoder.h \
						    ../dxt_compress/dxt_display.h \
						    ../dxt_compress/dxt_encoder.h \
						    ../dxt_compress/dxt_glsl.h \
						    ../dxt_compress/dxt_util.h
