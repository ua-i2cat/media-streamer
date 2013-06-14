CC            = gcc -std=gnu99
CXX           = g++
LINKER        = g++
CFLAGS        = -g -DHAVE_CONFIG_H -g -O2 -I. -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
CPPFLAGS      = -I.
CXXFLAGS      = -g -DHAVE_CONFIG_H -g -O2 -fPIC -Wno-multichar -Wno-deprecated -msse2
LDFLAGS       = -shared -Wl,--dynamic-list-data,-soname,vdecompress.so
LDFLAGS_TEST  = -Wl,--dynamic-list-data

LIBS         += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread
LIBS_TEST    += $(LIBS) -L./lib -lvdecompress

INC           = -I.
		  
TARGET        = lib/libvdecompress.so
TARGET_TEST   = bin/test

DOCS 	      = COPYRIGHT README REPORTING-BUGS

HEADERS	      =  

OBJS	      = video_decompress/libavcodec.o \
		debug.o \
		tile.o \
		utils/resource_manager.o \
		video.o \
		video_codec.o \
		video_decompress/null.o \
		video_decompress.o \

TEST_OBJS     = test/main.o \

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
	