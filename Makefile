CC            = gcc -std=gnu99
CXX           = g++
LINKER        = g++
CFLAGS        = -g -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
CPPFLAGS      = -I. 
CXXFLAGS      = -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2
LDFLAGS       = -shared -Wl,--dynamic-list-data,-soname
LDFLAGS_RTP   =-shared -Wl,--dynamic-list-data,-soname,librtp.so
LDFLAGS_ENC   =-shared -Wl,--dynamic-list-data,-soname,libvcompress.so
LDFLAGS_DEC   =-shared -Wl,--dynamic-list-data,-soname,libvdecompress.so
LDFLAGS_TEST  = -Wl,--dynamic-list-data

LIBS_RTP      += -lrt -ldl -lieee -lm
LIBS_ENC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread
LIBS_DEC      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread
LIBS          += $(LIBS_RTP) $(LIBS_ENC) $(LIBS_DEC)
	 	
LIBS_RTP_TEST += $(LIBS_RTP) -L./lib -lrtp
LIBS_ENC_TEST += $(LIBS_ENC) -L./lib -lvcompress
LIBS_DEC_TEST += $(LIBS_DEC) -L./lib -lvdecompress
LIBS_TEST     += $(LIBS) $(LIBS_RTP_TEST) $(LIBS_ENC_TEST) $(LIBS_DEC_TEST)

INC           = -I./src 
	  
TARGET_RTP    = lib/librtp.so
TARGET_ENC    = lib/libvcompress.so
TARGET_DEC    = lib/libvdecompress.so	
TARGET        = $(TARGET_RTP) $(TARGET_ENC) $(TARGET_DEC)	

TARGET_RTP_TEST   = bin/rtptest
TARGET_ENC_TEST   = bin/encodertest
TARGET_DEC_TEST   = bin/decodertest

DOCS 	      = COPYRIGHT README REPORTING-BUGS

HEADERS	      =  

OBJS	      = src/debug.o \
		src/tile.o \
		src/video.o \
		src/video_codec.o \

OBJS_RTP        += src/compat/drand48.o \
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

OBJS_RM         +=src/utils/resource_manager.o \
		src/utils/worker.o \

OBJS_ENC        += src/video_compress/none.o \
		src/video_compress/libavcodec.o \
		src/video_compress.o \

OBJS_DEC        += src/video_decompress/libavcodec.o \
		src/video_decompress/null.o \
		src/video_decompress.o \



TEST_OBJS_RTP = test/rtp.o \
TEST_OBJS_ENC = test/encoder.o \
TEST_OBJS_DEC = test/decoder.o \
TEST_OBJS     = $(TEST_OBJS_RTP) $(TEST_OBJS_ENC) $(TEST_OBJS_DEC)
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

rtp: $(TARGET_RTP)
$(TARGET_RTP): $(OBJS) $(OBJS_RTP) $(HEADERS)
	@if [ ! -d lib ]; then mkdir lib; fi
	$(LINKER) $(LDFLAGS_RTP) -o $(TARGET_RTP) $(OBJS) $(OBJS_RTP) $(LIBS_RTP)

rtptest: $(TARGET_RTP) $(TEST_OBJS)
	@if [ ! -d bin ]; then mkdir bin; fi
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS) $(LIBS_TEST) -o $(TARGET_TEST)

encoder: $(TARGET_ENC)
$(TARGET_ENC): $(OBJS) $(OBJS_RM) $(OBJS_ENC) $(HEADERS)
	@if [ ! -d lib ]; then mkdir lib; fi
	$(LINKER) $(LDFLAGS) $(LDFLAGS_ENC) -o $(TARGET_ENC) $(OBJS) $(OBJS_ENC) $(LIBS_ENC)

encodertest: $(TARGET_ENC) $(TEST_OBJS)
	@if [ ! -d bin ]; then mkdir bin; fi
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS) $(LIBS_TEST) -o $(TARGET_TEST)

decoder: $(TARGET_DEC)
$(TARGET_DEC): $(OBJS) $(OBJS_RM) $(OBJS_DEC) $(HEADERS)
	@if [ ! -d lib ]; then mkdir lib; fi
	$(LINKER) $(LDFLAGS) $(LDFLAGS_DEC) -o $(TARGET_DEC) $(OBJS) $(OBJS_DEC) $(LIBS_DEC)

decodertest: $(TARGET_DEC) $(TEST_OBJS)
	@if [ ! -d bin ]; then mkdir bin; fi
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS) $(LIBS_TEST) -o $(TARGET_TEST)
	

# -------------------------------------------------------------------------------------------------

# -------------------------------------------------------------------------------------------------
clean:
	-rm -f $(OBJS) $(TEST_OBJS) $(HEADERS) $(TARGET) $(TARGET_TEST)
	

