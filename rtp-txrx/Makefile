CC            = gcc -std=gnu99
CXX           = g++
LINKER        = g++
CFLAGS        = -g -DHAVE_CONFIG_H -g -I. -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
CPPFLAGS      = -I.
CXXFLAGS      = -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2
LDFLAGS       = -shared -Wl,--dynamic-list-data,-soname,librtp.so
LDFLAGS_TEST  = -Wl,--dynamic-list-data

LIBS         += -lrt -ldl -lieee -lm
LIBS_TEST    += $(LIBS) -L./lib -lrtp

INC           = -I.
		  
TARGET        = lib/librtp.so
TARGET_TEST   = bin/test

DOCS 	      = COPYRIGHT README REPORTING-BUGS

HEADERS	      =  

OBJS	      = compat/drand48.o \
		crypto/crypt_des.o \
		crypto/crypt_aes.o \
		crypto/crypt_aes_impl.o \
		crypto/md5.o \
		compat/gettimeofday.o \
		crypto/random.o \
		debug.o \
		tv.o \
		perf.o \
		tfrc.o \
		ntp.o \
		pdb.o \
		compat/inet_pton.o \
		compat/inet_ntop.o \
		compat/vsnprintf.o \
		rtp/pbuf.o \
		rtp/rtp_callback.o \
		rtp/net_udp.o \
		rtp/rtp.o \
		rtp/rtpdec.o \
		rtp/rtpenc.o \
		rtp/rtpdec_h264.o \
		rtp/rtpenc_h264.o \
		
		

TEST_OBJS     = test/main.o \
				test/video.o \
				test/video_codec.o \
				test/tile.o \

# -------------------------------------------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	@if [ ! -d lib ]; then mkdir lib; fi
	$(LINKER) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

configure-messages:
	@echo ""

test: $(TARGET) $(TEST_OBJS)
	@if [ ! -d bin ]; then mkdir bin; fi
	
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS) $(LIBS_TEST) -o $(TARGET_TEST)
	

# -------------------------------------------------------------------------------------------------

# -------------------------------------------------------------------------------------------------
clean:
	-rm -f $(OBJS) $(TEST_OBJS) $(HEADERS) $(TARGET) $(TARGET_TEST)
	