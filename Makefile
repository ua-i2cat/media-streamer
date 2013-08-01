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
LIBS_ENC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut
LIBS_DEC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut
LIBS	      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -lGLEW -lGL -lglut
	 	
LIBS_RTP_TEST += $(LIBS_RTP) -L./lib -lrtp
LIBS_ENC_TEST += $(LIBS_ENC) -L./lib -lvcompress
LIBS_DEC_TEST += $(LIBS_DEC) -L./lib -lvdecompress

LIBS_TEST     += $(LIBS) -L./lib -lrtp -lvcompress -lvdecompress -lavformat

INC           = -I./src -I$(HOME)/ffmpeg_build/include -I./
	  
TARGET_RTP    = lib/librtp.so
TARGET_ENC    = lib/libvcompress.so
TARGET_DEC    = lib/libvdecompress.so
TARGETS       = $(TARGET_RTP) $(TARGET_ENC) $(TARGET_DEC)

TESTS = $(addprefix bin/, rtp encoder decoder ug_ug vlc_vlc vlc_ug ug_vlc enc_tx rx_dec enc_dec 2in2out)

TRANSMITTER = $(addprefix bin/, test_transmitter)

RECIEVER = $(addprefix bin/, test)

DOCS 	      = COPYRIGHT README REPORTING-BUGS

# -----------------------------------------------------------------------------

OBJS_LINK 	+= dxt_compress/dxt_util.o \
				dxt_compress/dxt_decoder.o \
				dxt_compress/dxt_encoder.o

# -----------------------------------------------------------------------------

OBJS_RTP 	 += src/tv.o \
				src/perf.o \
				src/tfrc.o \
				src/ntp.o \
				src/pdb.o \
				src/debug.o \
				src/compat/gettimeofday.o \
				src/compat/inet_pton.o \
				src/compat/inet_ntop.o \
				src/compat/vsnprintf.o \
				src/compat/drand48.c \
				src/utils/list.o \


OBJS_RTP	+= $(patsubst %.c, %.o,	$(wildcard src/crypto/*.c))
OBJS_RTP	+= $(patsubst %.c, %.o,	$(wildcard src/rtp/*.c)) 

# -----------------------------------------------------------------------------

OBJS_ENC_PRE     += src/video.o \
				src/gl_context.o \
				src/glx_common.o \
				src/x11_common.o \
				src/video_compress.o \
				src/debug.o \
				src/video_codec.o \
				src/module.o \
				src/messaging.o \
				src/tile.o \
				src/compat/platform_spin.o \
				src/compat/platform_semaphore.o \


OBJS_ENC_PRE	+= $(patsubst %.c, %.o,	$(wildcard src/video_compress/*.c))
OBJS_ENC_PRE	+= $(patsubst %.c, %.o,	$(wildcard src/utils/*.c))
OBJS_ENC_PRE	+= $(patsubst %.cpp, %.o,	$(wildcard src/utils/*.cpp))

OBJS_ENC_EX	= src/video_compress/fastdxt.o \
			src/video_compress/jpeg.o
				
OBJS_ENC	= $(filter-out $(OBJS_ENC_EX), $(OBJS_ENC_PRE))

# -----------------------------------------------------------------------------


OBJS_DEC_PRE     += src/video.o \
				src/video_decompress.o \
				src/debug.o \
				src/video_codec.o \
				src/module.o \
				src/messaging.o \
				src/tile.o \
				src/compat/platform_spin.o \
				src/compat/platform_semaphore.o \


OBJS_DEC_PRE	+= $(patsubst %.c, %.o,	$(wildcard src/video_decompress/*.c))
OBJS_DEC_PRE	+= $(patsubst %.c, %.o,	$(wildcard src/utils/*.c))
OBJS_DEC_PRE	+= $(patsubst %.cpp, %.o,	$(wildcard src/utils/*.cpp))

OBJS_DEC_EX	= src/video_decompress/jpeg.o \
			src/video_decompress/jpeg_to_dxt.o \


OBJS_DEC	= $(filter-out $(OBJS_DEC_EX), $(OBJS_DEC_PRE))

# -----------------------------------------------------------------------------

OBJS_C		  = $(patsubst %.c, %.o ,	$(wildcard src/*.c) $(wildcard src/*/*.c))
OBJS_CPP	  =	$(patsubst %.cpp, %.o,	$(wildcard src/*.cpp) $(wildcard src/*/*.cpp))

OBJS_EXCLUDE  = src/lib_common.o

OBJS		  = $(filter-out $(OBJS_EXCLUDE), $(OBJS_C) $(OBJS_CPP))

OBJS_TEST     = $(patsubst %.c, %.o,	$(wildcard tests/*.c) $(wildcard tests/*/*.c))

OBJS_RECIEVER = $(addprefix reciever/, participants.o reciever.o test.o)

OBJS_TRANSMITTER = $(addprefix reciever/, participants.o transmitter.o test_transmitter.o)

# -------------------------------------------------------------------------------------------------

all: build $(TARGETS)

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

configure-messages:
	@echo ""

tests: test

transmitter: build $(TARGETS) $(OBJS_TRANSMITTER) $(TRANSMITTER)

reciever: dxt build $(TARGETS) $(OBJS_RECIEVER) $(RECIEVER)

test: build $(TARGETS) $(TESTS)

build:
	@mkdir -p lib
	@mkdir -p bin

rtp: $(TARGET_RTP)
encoder: $(TARGET_ENC)
decoder: $(TARGET_DEC)

dxt:
	cd dxt_compress; make

$(TARGET_RTP): $(OBJS_RTP)
	$(LINKER) $(LDFLAGS_RTP) -o $(TARGET_RTP) $+ $(LIBS_RTP)

$(TARGET_ENC): $(OBJS_ENC)
	$(LINKER) $(LDFLAGS) $(LDFLAGS_ENC) -o $(TARGET_ENC) $+ $(LIBS_ENC)

$(TARGET_DEC): $(OBJS_DEC)
	$(LINKER) $(LDFLAGS) $(LDFLAGS_DEC) -o $(TARGET_DEC) $+ $(LIBS_DEC)

bin/%: tests/%.o $(OBJS) $(HEADERS)
	$(LINKER) $(LDFLAGS_TEST) $(INC) $+ -o $@ $(LIBS_TEST)
	
$(RECIEVER): $(OBJS_RECIEVER) $(OBJS_LINK)
	$(LINKER) $(LDFLAGS_TEST) $(INC) $+ -o $@ $(LIBS_TEST)
	
$(TRANSMITTER): $(OBJS_TRANSMITTER) $(OBJS)
	$(LINKER) $(LDFLAGS_TEST) $(INC) $+ -o $@ $(LIBS_TEST)


# -------------------------------------------------------------------------------------------------

clean:
	rm -f $(OBJS) $(OBJS_TEST) $(HEADERS) $(TARGETS) $(TESTS) $(OBJS_RECIEVER) $(OBJS_TRANSMITTER) $(RECIEVER) $(TRANSMITTER)
	cd dxt_compress; make clean

