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
LIBS_ENC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm
LIBS_DEC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm
LIBS	      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm
	 	
LIBS_RTP_TEST += $(LIBS_RTP) -L./lib -lrtp
LIBS_ENC_TEST += $(LIBS_ENC) -L./lib -lvcompress
LIBS_DEC_TEST += $(LIBS_DEC) -L./lib -lvdecompress

LIBS_TEST     += $(LIBS) -L./lib -lrtp -lvcompress -lvdecompress -lavformat

INC           = -I./src -I$(HOME)/ffmpeg_build/include
	  
TARGET_RTP  = lib/librtp.so
TARGET_ENC  = lib/libvcompress.so
TARGET_DEC  = lib/libvdecompress.so
TARGET_LIBS = $(TARGET_RTP) $(TARGET_ENC) $(TARGET_DEC)

TARGET_RX   = bin/test
TARGET_TX   = bin/test_transmitter

TARGET_TEST = $(addprefix bin/, rtp encoder decoder ug_ug vlc_vlc vlc_ug ug_vlc enc_tx rx_dec enc_dec 2in2out)

TARGET_ALL  = $(TARGET_LIBS) $(TARGET_RX) $(TARGET_TX) $(TARGET_TEST)

DOCS 	    = COPYRIGHT README REPORTING-BUGS

SRCS_RTP  = $(wildcard src/*.c src/compat/*.c src/crypto/*.c src/rtp/*.c)
SRCS_RM   = $(wildcard src/utils/*.cpp)
SRCS_ENC  = $(wildcard src/video_compress/*.c)
SRCS_DEC  = $(wildcard src/video_decompress/*.c)

SRCS_RX   = $(addprefix reciever/, participants.c reciever.c test.c)
SRCS_TX   = $(addprefix reciever/, participants.c transmitter.c test_transmitter.c)


SRCS_TEST = $(wildcard tests/*.c)

# src/lib_common.o must be excluded (it does not compile and, apparently, it's not used)
OBJS_RTP  = $(filter-out src/lib_common.o, $(patsubst %.c, %.o, $(SRCS_RTP)))
OBJS_RM   = $(patsubst %.cpp, %.o, $(SRCS_RM))
OBJS_ENC  = $(patsubst %.c, %.o, $(SRCS_ENC))
OBJS_DEC  = $(patsubst %.c, %.o, $(SRCS_DEC))

OBJS_RX   = $(patsubst %.c, %.o, $(SRCS_RX))
OBJS_TX   = $(patsubst %.c, %.o, $(SRCS_TX))

OBJS_TEST = $(patsubst %.c, %.o, $(SRCS_TEST))

OBJS_ALL  = $(OBJS_RTP) $(OBJS_RM) $(OBJS_ENC) $(OBJS_DEC) $(OBJS_RX) $(OBJS_TX) $(OBJS_TEST)

# -------------------------------------------------------------------------------------------------

all: build $(TARGET_ALL) doc

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

configure-messages:
	@echo ""

rtp: build $(TARGET_RTP)

encoder: build $(TARGET_ENC)

decoder: build $(TARGET_DEC)

libs: build $(TARGET_RTP) $(TARGET_ENC) $(TARGET_DEC)

transmitter: build $(TARGET_TX)

reciever: build $(TARGET_RX)

tests: test

test: build $(TARGET_TEST)

build:
	@mkdir -p lib
	@mkdir -p bin

$(TARGET_RTP): $(OBJS_RTP) $(OBJS_RM) $(HEADERS)
	$(LINKER) $(LDFLAGS_RTP) -o $(TARGET_RTP) $+ $(LIBS_RTP)

$(TARGET_ENC): $(OBJS_ENC) $(OBJS_RM) $(HEADERS)
	$(LINKER) $(LDFLAGS_ENC) -o $(TARGET_ENC) $+ $(LIBS_ENC)

$(TARGET_DEC): $(OBJS_DEC) $(OBJS_RM) $(HEADERS)
	$(LINKER) $(LDFLAGS_DEC) -o $(TARGET_DEC) $+ $(LIBS_DEC)

# tests
bin/%: tests/%.o $(TARGET_LIBS) $(HEADERS)
	$(LINKER) $(LDFLAGS_TEST) $(INC) $< $(HEADERS) -o $@ $(LIBS_TEST)

$(TARGET_RX): $(TARGET_LIBS) $(OBJS_RX) $(HEADERS)
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(OBJS_RX) $(HEADERS) -o $@ $(LIBS_TEST)
	
$(TARGET_TX): $(TARGET_LIBS) $(OBJS_TX) $(HEADERS)
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(OBJS_TX) $(HEADERS) -o $@ $(LIBS_TEST)


doc:
	doxygen

# -------------------------------------------------------------------------------------------------

clean:
	rm -f $(OBJS_ALL) $(HEADERS) $(TARGET_ALL)
	rm -rf doc
