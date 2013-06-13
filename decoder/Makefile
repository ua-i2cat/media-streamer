AR            = ar
RANLIB        = ranlib
CC            = gcc -std=gnu99
CXX           = g++
LINKER        = g++
CFLAGS        = -g -DHAVE_CONFIG_H -g -O2 -I. -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -I/usr/local/include    -DPATH_PREFIX=/usr/local -DLIB_DIR=${exec_prefix}/lib -msse2
CPPFLAGS      =  -I.
CXXFLAGS      = -g -DHAVE_CONFIG_H -g -O2 -fPIC -DPATH_PREFIX=/usr/local -DLIB_DIR=${exec_prefix}/lib -Wno-multichar -Wno-deprecated -msse2
LDFLAGS      =  -shared -Wl,--dynamic-list-data,-soname,decompress.so
LIBS         += -lrt -lpthread -ldl -pthread -L/usr/local/lib -lavcodec -ldl -lva -lx264 -lvpx -lrtmp -lgnutls -lopencore-amrwb -lopencore-amrnb -lass -lz -lrt -lavutil -lieee
INC           = -I.
		  
TARGET        = lib/decompress.so

DOCS 	      = COPYRIGHT README REPORTING-BUGS

#PREFIX = $HOME/local
#prefix = $(PREFIX)
#exec_prefix = ${prefix}
#INSTALL = /usr/bin/install -c
#bindir = ${exec_prefix}/bin
#libdir = ${exec_prefix}/lib
#datadir = ${datarootdir}
#docdir = ${datarootdir}/doc/${PACKAGE_TARNAME}/ultragrid/
#datarootdir = ${prefix}/share
#uv_datadir = ${datarootdir}/ultragrid/

HEADERS	      =  

OBJS	      = video_decompress/libavcodec.o \
		debug.o \
		tile.o \
		video.o \
		video_codec.o \
		video_decompress/null.o \
		video_decompress.o \
		utils/resource_manager.o \
		

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


# -------------------------------------------------------------------------------------------------

# -------------------------------------------------------------------------------------------------
clean:
	-rm -f $(OBJS) $(HEADERS) $(TARGET)
#	-rm -f $(TEST_OBJS) test/run_tests
#	-rm -f ag_plugin/uvReceiverService.zip ag_plugin/uvSenderService.zip
#	-rm -rf $(BUNDLE)
#	-rm -rf $(PERF) src/uv_perf.o
#	-rm -rf $(HD_RUM_TRANSCODE_OBJS)
#	-rm -rf   
#	-rm -rf bin/import_control_keyboard
#	[ ! -f gpujpeg/Makefile ] || make -C gpujpeg/ clean
#	[ -z "" ] || make -C / clean
#	if [ -f "gui/QT/Makefile" ]; then make -C gui/QT/ clean; fi

#distclean: clean
#	-rm -f Makefile config.status config.cache config.log src/config.h tags
#	[ ! -f gpujpeg/Makefile ] || make -C gpujpeg/ distclean
#	[ -z "" ] || make -C / distclean

#etags:
#	etags src/*.[ch] src/*/*.[ch]

#ctags:
#	ctags src/*.[ch] src/*/*.[ch]

#release:
#	cvs tag release-`cat VERSION | sed "s/\./-/g"`

#bundle: bundle-nolib
#	@if `which dylibbundler`; then \
#		dylibbundler -of -p @executable_path/../libs/ -od -b -x $(BUNDLE)/Contents/MacOS/uv -d $(BUNDLE)/Contents/libs/; \
#		for n in $(BUNDLE)/Contents/MacOS/uv $(BUNDLE)/Contents/libs/*; do echo $$n; while $$( otool -L $$n | grep -q @rpath) ; \
#			do NAME=$$( otool -L $$n | grep @rpath| tail -n 1|cut -f 1 -d\ ); install_name_tool -change $$NAME $${NAME/@rpath/@executable_path\/..\/libs} $$n ; done; done ; \
#	else \
#		echo "WARNING: Cannot find dylibbundler. We cannot put libraries into a bundle so it probably won't be portable."; \
#	fi

#	#echo -n "UltraGrid" > $(BUNDLE)/Contents/PkgInfo


#install: all
#	$(INSTALL) -d -m 755 $(DESTDIR)/$(bindir)
#	$(INSTALL) -m 755 bin/uv $(DESTDIR)/$(bindir)
#	if [ -f "$(GUI_TARGET)" ]; then $(INSTALL) -m 755 $(GUI_TARGET) $(DESTDIR)/$(bindir); fi
#	if [ -n "" ]; then\
#		$(INSTALL) -d -m 755 $(DESTDIR)/$(libdir)/ultragrid;\
#		$(INSTALL) -m 644  $(DESTDIR)/$(libdir)/ultragrid/;\
#	fi
#	$(INSTALL) -d -m 755 $(DESTDIR)/$(uv_datadir)
#	$(INSTALL) -m 755 data/ultragrid-bugreport-collect.sh $(DESTDIR)/$(uv_datadir)
#	$(INSTALL) -d -m 755 $(DESTDIR)/$(docdir)
#	$(INSTALL) -m 644 $(DOCS) $(DESTDIR)/$(docdir)

#uninstall:
#	$(RM) $(DESTDIR)/$(bindir)/uv
#	if [ -n "" ]; then for n in ; do $(RM) $(DESTDIR)/$(libdir)/ultragrid/`basename $$n`; done; fi
#	$(RM) $(DESTDIR)/$(uv_datadir)/ultragrid-bugreport-collect.sh
#	for n in $(DOCS); do $(RM) $(DESTDIR)/$(docdir)/$$n; done; fi
#	$(RM) $(DESTDIR)/$(uv_datadir)/ultragrid-bugreport-collect.sh
#	if [ -f "$(GUI_TARGET)" ]; then $(RM) $(DESTDIR)/$(bindir)/`basename $(GUI_TARGET)`; fi
