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
LIBS	      += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread
	 	
LIBS_RTP_TEST += $(LIBS_RTP) -L./lib -lrtp
LIBS_ENC_TEST += $(LIBS_ENC) -L./lib -lvcompress
LIBS_DEC_TEST += $(LIBS_DEC) -L./lib -lvdecompress

LIBS_TEST     += $(LIBS) -L./lib -lrtp -lvcompress -lvdecompress

INC           = -I./src 
	  
TARGET_RTP    = lib/librtp.so
TARGET_ENC    = lib/libvcompress.so
TARGET_DEC    = lib/libvdecompress.so	
TARGET        = $(TARGET_RTP) $(TARGET_ENC) $(TARGET_DEC)	

TARGET_RTP_TEST   = bin/rtptest
TARGET_ENC_TEST   = bin/encodertest
TARGET_DEC_TEST   = bin/decodertest
TARGET_UG_UG_TEST = bin/ugugtest
TARGET_VLC_VLC_TEST = bin/vlcvlctest
TARGET_VLC_UG_TEST = bin/vlcugtest
TARGET_UG_VLC_TEST = bin/ugvlctest

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



TEST_OBJS_RTP = tests/rtp.o
TEST_OBJS_ENC = tests/encoder.o
TEST_OBJS_DEC = tests/decoder.o
TEST_OBJS_UG_UG = tests/ug_ug.o
TEST_OBJS_VLC_VLC = tests/vlc_vlc.o
TEST_OBJS_VLC_UG = tests/vlc_ug.o
TEST_OBJS_UG_VLC = tests/ug_vlc.o

TEST_OBJS     = $(TEST_OBJS_RTP) $(TEST_OBJS_ENC) $(TEST_OBJS_DEC) $(TEST_OBJS_UG_UG)
# -------------------------------------------------------------------------------------------------
all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

configure-messages:
	@echo ""

test: rtptest encodertest decodertest ugugtest vlcvlctest vlcugtest ugvlctest

rtp: $(TARGET_RTP)
$(TARGET_RTP): $(OBJS) $(OBJS_RTP) $(HEADERS)
	@mkdir -p lib
	$(LINKER) $(LDFLAGS_RTP) -o $(TARGET_RTP) $(OBJS) $(OBJS_RTP) $(LIBS_RTP)

rtptest: $(TARGET_RTP) $(TEST_OBJS_RTP)
	@mkdir -p bin
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS_RTP) $(LIBS_RTP_TEST) -o $(TARGET_RTP_TEST)

encoder: $(TARGET_ENC)
$(TARGET_ENC): $(OBJS) $(OBJS_RM) $(OBJS_ENC) $(HEADERS)
	@mkdir -p lib
	$(LINKER) $(LDFLAGS) $(LDFLAGS_ENC) -o $(TARGET_ENC) $(OBJS) $(OBJS_RM) $(OBJS_ENC) $(LIBS_ENC)

encodertest: $(TARGET_ENC) $(TEST_OBJS_ENC)
	@mkdir -p bin
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS_ENC) $(LIBS_ENC_TEST) -o $(TARGET_ENC_TEST)

decoder: $(TARGET_DEC)
$(TARGET_DEC): $(OBJS) $(OBJS_RM) $(OBJS_DEC) $(HEADERS)
	@mkdir -p lib
	$(LINKER) $(LDFLAGS) $(LDFLAGS_DEC) -o $(TARGET_DEC) $(OBJS) $(OBJS_RM) $(OBJS_DEC) $(LIBS_DEC)

decodertest: $(TARGET_DEC) $(TEST_OBJS_DEC)
	@mkdir -p bin
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS_DEC) $(LIBS_DEC_TEST) -o $(TARGET_DEC_TEST)
	

ugugtest: $(TARGET_DEC) $(TARGET_ENC) $(TARGET_RTP) $(TEST_OBJS_UG_UG)
	@mkdir -p bin
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS_UG_UG) $(LIBS_TEST) -o $(TARGET_UG_UG_TEST)

vlcvlctest: $(TARGET_DEC) $(TARGET_ENC) $(TARGET_RTP) $(TEST_OBJS_VLC_VLC)
	@mkdir -p bin
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS_VLC_VLC) $(LIBS_TEST) -o $(TARGET_VLC_VLC_TEST)

vlcugtest: $(TARGET_DEC) $(TARGET_ENC) $(TARGET_RTP) $(TEST_OBJS_VLC_UG)
	@mkdir -p bin
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS_VLC_UG) $(LIBS_TEST) -o $(TARGET_VLC_UG_TEST)

ugvlctest: $(TARGET_DEC) $(TARGET_ENC) $(TARGET_RTP) $(TEST_OBJS_UG_VLC)
	@mkdir -p bin
	$(LINKER) $(LDFLAGS_TEST) $(INC) $(TEST_OBJS_UG_VLC) $(LIBS_TEST) -o $(TARGET_UG_VLC_TEST)
# -------------------------------------------------------------------------------------------------

# -------------------------------------------------------------------------------------------------
clean:
	rm -f $(OBJS) $(OBJS_RTP) $(OBJS_ENC) $(OBJS_DEC) $(OBJS_RM) $(TEST_OBJS) $(HEADERS) $(TARGET) $(TARGET_TEST)
