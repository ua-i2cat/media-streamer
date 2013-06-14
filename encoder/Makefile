CC            = gcc -std=gnu99
CXX           = g++
LINKER        = g++
CFLAGS        = -g -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
CPPFLAGS      = -I. 
CXXFLAGS      = -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2
LDFLAGS       = -shared -Wl,--dynamic-list-data,-soname,vcompress.so
LDFLAGS_TEST  = -Wl,--dynamic-list-data

LIBS         += -lrt -lpthread -ldl -lavcodec -lavutil -lieee -lm -pthread

LIBS_TEST    += $(LIBS) -L./lib -lvcompress

INC           = -I.
		  
TARGET        = lib/libvcompress.so
TARGET_TEST   = bin/test

DOCS 	      = COPYRIGHT README REPORTING-BUGS

HEADERS	      =  

OBJS	      = utils/resource_manager.o \
		utils/worker.o \
		debug.o \
		tile.o \
		video.o \
		video_codec.o \
		video_compress/none.o \
		video_compress/libavcodec.o \
		video_compress.o \

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
	

