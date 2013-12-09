lib_LTLIBRARIES = libiomanager.la


AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
AM_CXXFLAGS = -O2 -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2


libiomanager_la_LDFLAGS = -version-info 0:1:0 -L. -L.. -L../src -lpthread -lrtp -lvcompress -lvdecompress -lliveMedia -lBasicUsageEnvironment -lUsageEnvironment -lgroupsock
libiomanager_la_CFLAGS = $(AM_CFLAGS) -I. -I.. -I../src -I../src/rtp -I../src/utils -I../src/audio -I/usr/include/groupsock -I/usr/include/liveMedia -I/usr/include/BasicUsageEnvironment -I/usr/include/UsageEnvironment
libiomanager_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I.. -I../src -I../src/rtp -I../src/utils -I../src/audio -I/usr/include/groupsock -I/usr/include/liveMedia -I/usr/include/BasicUsageEnvironment -I/usr/include/UsageEnvironment
libiomanager_la_SOURCES = stream.c \
						  participants.c \
						  video_data.c \
						  transmitter.c \
						  receiver.c \
						  BasicRTSPOnlyServer.cpp \
						  BasicRTSPOnlySubsession.cpp \
						  c_basicRTSPOnlyServer.cpp \
						  ../dxt_compress/dxt_util.c \
						  ../dxt_compress/dxt_decoder.c \
						  ../dxt_compress/dxt_encoder.c

ugincludedir = $(includedir)/io_mngr
uginclude_HEADERS =	receiver.h \
					config_unix.h \
					participants.h \
					transmitter.h \
					stream.h \
					video_data.h \
					commons.h \
					audio_processor.h \
					audio_config.h \
					circular_queue.h \
					../src/video_data_frame.h \
					../src/video.h \
					../src/video_codec.h \
					../src/types.h \
					../src/module.h \
					../src/transmit.h \
					../src/messaging.h \
					../src/video_frame.h \
					../src/debug.h \
					../src/rtp/rtpdec.h \
					../src/rtp/pbuf.h \
					../src/utils/list.h \
					../src/rtp/rtp.h \
					../src/audio/audio.h \
					../src/config.h \
					BasicRTSPOnlyServer.hh \
					BasicRTSPOnlySubsession.hh \
					c_basicRTSPOnlyServer.h
