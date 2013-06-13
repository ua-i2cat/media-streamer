CC            = gcc -std=gnu99
CXX           = g++
LINKER        = g++
CFLAGS        = -g -DHAVE_CONFIG_H -g -O2 -I. -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -I/usr/local/include   -DPATH_PREFIX=/usr/local -DLIB_DIR=/usr/local/lib -msse2
CXXFLAGS      = -g -DHAVE_CONFIG_H -g -O2 -fPIC -DPATH_PREFIX=/usr/local -DLIB_DIR=/usr/local/lib -Wno-multichar -Wno-deprecated -msse2
LDFLAGS       = -Wl,--dynamic-list-data
LIBS         += -lrt -lpthread -L/usr/local/lib -L/usr/local/lib -lieee -lm -pthread
INC           = -I. -Isrc
TARGET        = bin/uv

OBJS =  src/debug.o \
		src/host.o \
		src/perf.o \
		src/ntp.o \
		src/pdb.o \
		src/tile.o \
		src/tv.o \
		src/transmit.o \
		src/tfrc.o \
		src/rtp/pbuf.o \
		src/rtp/decoders.o \
		src/rtp/audio_decoders.o \
		src/rtp/ptime.o \
		src/rtp/net_udp.o \
		src/rtp/ll.o \
		src/rtp/rtp.o \
		src/rtp/rtp_callback.o \
		src/audio/audio.o \
		src/audio/audio_capture.o \
		src/audio/audio_playback.o \
		src/audio/capture/none.o \
		src/audio/codec.o \
		src/audio/codec/dummy_pcm.o \
		src/audio/export.o \
		src/audio/playback/none.o \
		src/audio/resampler.o \
		src/audio/utils.o \
		src/compat/drand48.o \
		src/compat/gettimeofday.o \
		src/compat/inet_ntop.o \
		src/compat/inet_pton.o \
		src/compat/platform_semaphore.o \
		src/compat/platform_time.o \
		src/compat/usleep.o \
		src/crypto/crypt_aes.o \
		src/crypto/crypt_aes_impl.o \
		src/crypto/crypt_des.o \
		src/crypto/md5.o \
		src/crypto/random.o \
		src/utils/list.o \
		src/utils/packet_counter.o \
		src/utils/resource_manager.o \
		src/utils/ring_buffer.o \
		src/utils/worker.o \
		src/video.o \
		src/video_codec.o \
		src/video_capture.o \
		src/video_capture/null.o \
		src/video_capture/net.o \
		src/video_compress.o \
		src/video_compress/none.o \
		src/video_decompress.o \
		src/video_decompress/null.o \
		src/video_display.o \
		src/video_display/null.o \
		src/video_display/net.o \
		src/video_export.o \
# -------------------------------------------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS) src/main.o
	@if [ ! -d bin ]; then mkdir bin; fi
	$(LINKER) $(LDFLAGS) $(OBJS) src/main.o $(LIBS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

src/utils/autorelease_pool.o: src/utils/autorelease_pool.m src/utils/autorelease_pool.h
	$(CC) $(CFLAGS) $(INC) -x objective-c -c $< -o $@

src/utils/packet_counter.o: src/utils/packet_counter.cpp src/utils/packet_counter.h
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

# -------------------------------------------------------------------------------------------------
clean:
	-rm -f $(OBJS) src/main.o $(TARGET) src/version.h
