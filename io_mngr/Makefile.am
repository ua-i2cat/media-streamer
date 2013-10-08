lib_LTLIBRARIES = libiomanager.la


AM_CFLAGS = -std=gnu99 -g -lm -DHAVE_CONFIG_H -g -fPIC -pipe -W -Wall -Wcast-qual -Wcast-align -Wbad-function-cast -Wmissing-prototypes -Wmissing-declarations -msse2
AM_CXXFLAGS = -O2 -g -DHAVE_CONFIG_H -g -fPIC -Wno-multichar -Wno-deprecated -msse2


libiomanager_la_LDFLAGS = -version-info 0:1:0 -lpthread -lrtp -lvcompress -lvdecompress
libiomanager_la_CFLAGS = $(AM_CFLAGS) -I. -I.. -I../src
libiomanager_la_CXXFLAGS = $(AM_CXXFLAGS) -I. -I.. -I../src
libiomanager_la_SOURCES = participants.c \
						  receiver.c \
						  transmitter.c

ugincludedir = $(includedir)/io_mngr
nobase_uginclude_HEADERS =	receiver.h \
							config.h \
							participants.h \
							transmitter.h
