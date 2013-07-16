CC            = gcc -std=gnu99
CXX           = g++
LINKER        = g++
CFLAGS        = -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
CPPFLAGS      = -I. 
CXXFLAGS      = -g -lm -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2
LDFLAGS       = -shared -Wl,--dynamic-list-data,--as-needed,-gc-sections,-soname
LDFLAGS_RTP   =-shared  -Wl,--dynamic-list-data,--as-needed,-gc-sections,-soname,librtp.so
LDFLAGS_ENC   =-shared  -Wl,--dynamic-list-data,--as-needed,-gc-sections,-soname,libvcompress.so
LDFLAGS_DEC   =-shared  -Wl,--dynamic-list-data,--as-needed,-gc-sections,-soname,libvdecompress.so
LDFLAGS_TEST  = -Wl,--dynamic-list-data,--as-needed

LIBS_RTP      += -lrt -ldl -lieee -lm
LIBS_ENC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread
LIBS_DEC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread
LIBS	      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread
	 	
LIBS_RTP_TEST += $(LIBS_RTP) -L./lib -lrtp
LIBS_ENC_TEST += $(LIBS_ENC) -L./lib -lvcompress
LIBS_DEC_TEST += $(LIBS_DEC) -L./lib -lvdecompress

LIBS_TEST     += $(LIBS) -L./lib -lrtp -lvcompress -lvdecompress -lavformat

INC           = -I./src 
	  
TARGET_RTP    = lib/librtp.so
TARGET_ENC    = lib/libvcompress.so
TARGET_DEC    = lib/libvdecompress.so
TARGETS       = $(TARGET_RTP) $(TARGET_ENC) $(TARGET_DEC)	

TESTS = $(addprefix bin/, rtp encoder decoder ug_ug vlc_vlc vlc_ug ug_vlc enc_tx rx_dec enc_dec 2in2out)

DOCS 	      = COPYRIGHT README REPORTING-BUGS

HEADERS	      =  

#OBJS	      = src/debug.o \
				src/tile.o \
				src/video.o \
				src/video_codec.o \

OBJS_RTP 	 += src/compat/drand48.o \
				src/crypto/crypt_des.o \
				src/crypto/crypt_aes.o \
				src/crypto/crypt_aes_impl.o \
				src/crypto/md5.o \
				src/compat/gettimeofday.o \
				src/crypto/random.o \
				src/tv.o \
				src/perf.o \
				src/tfrc.o \
				src/ntp.o \
				src/pdb.o \
				src/compat/inet_pton.o \
				src/compat/inet_ntop.o \
				src/compat/vsnprintf.o \
				src/rtp/pbuf.o \
				src/rtp/rtp_callback.o \
				src/rtp/net_udp.o \
				src/rtp/rtp.o \
				src/rtp/rtpdec.o \
				src/rtp/rtpenc.o \
				src/rtp/rtpdec_h264.o \
				src/rtp/rtpenc_h264.o \

OBJS_RM 	 += src/utils/resource_manager.o \
				src/utils/worker.o \

OBJS_ENC     += src/video_compress/none.o \
				src/video_compress/libavcodec.o \
				src/video_compress.o \

OBJS_DEC     += src/video_decompress/libavcodec.o \
				src/video_decompress/null.o \
				src/video_decompress.o \

OBJS_C		  = $(patsubst %.c, %.o ,	$(wildcard src/*.c) $(wildcard src/*/*.c))
OBJS_CPP	  =	$(patsubst %.cpp, %.o,	$(wildcard src/*.cpp) $(wildcard src/*/*.cpp))

OBJS_TEST     = $(patsubst %.c, %.o,	$(wildcard tests/*.c) $(wildcard tests/*/*.c))

OBJS_EXCLUDE  = src/lib_common.o

OBJS		  = $(filter-out $(OBJS_EXCLUDE), $(OBJS_C) $(OBJS_CPP))

# -------------------------------------------------------------------------------------------------

all: build $(TARGETS)

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

configure-messages:
	@echo ""

tests: test

test: build $(TESTS)

build:
	@mkdir -p lib
	@mkdir -p bin

rtp: $(TARGET_RTP)
encoder: $(TARGET_ENC)
decoder: $(TARGET_DEC)

$(TARGET_RTP): $(OBJS_RTP) $(HEADERS)
	$(LINKER) $(LDFLAGS_RTP) -o $(TARGET_RTP) $+ $(LIBS_RTP)

$(TARGET_ENC): $(OBJS_ENC) $(HEADERS)
	$(LINKER) $(LDFLAGS) $(LDFLAGS_ENC) -o $(TARGET_ENC) $+ $(LIBS_ENC)

$(TARGET_DEC): $(OBJS_DEC) $(HEADERS)
	$(LINKER) $(LDFLAGS) $(LDFLAGS_DEC) -o $(TARGET_DEC) $+ $(LIBS_DEC)

bin/%: tests/%.o $(OBJS) $(HEADERS)
	$(LINKER) $(LDFLAGS_TEST) $(INC) $+ -o $@ $(LIBS_TEST)


# -------------------------------------------------------------------------------------------------

clean:
	rm -f $(OBJS) $(HEADERS) $(TARGETS) $(TESTS)
