## Process this file with automake to produce Makefile.in
#

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(TRUDP_CFLAGS)

AM_CFLAGS = \
	 -g -Wall \
	 -fPIC \
	 -I$(HOME)/Projects/teonet/embedded/libtrudp/src \
	 -I/usr/include/libtrudp
	
#	 -std=c11 
#	 -I$(HOME)/Projects/teonet/src/ 

AM_CXX = clang++
AM_CXXFLAGS = \
         -g -Wall \
	 -fPIC \
	 -I$(HOME)/Projects/teonet/embedded/libtrudp/src \
	 -I/usr/include/libtrudp

#	 -std=c++11 
#	 -I$(HOME)/Projects/teonet/src/

## Debug info
if DEBUG
  AM_CFLAGS += -g3 -O0
  AM_CXXFLAGS += -g3 -O0
else
  AM_CFLAGS += -O2
  AM_CXXFLAGS += -O2
endif

AM_LDFLAGS = 

LIBS =

lib_LTLIBRARIES = libtrudp.la

libtrudp_la_SOURCES = \
	hash.c \
	map.c \
	packet.c \
	packet_queue.c \
	queue.c \
	trudp.c \
	trudp_stat.c \
	udp.c \
	utils.c \
	write_queue.c
	
libtrudpincludedir=$(includedir)
	
nobase_libtrudpinclude_HEADERS = \
	trudp.h

libtrudp_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBRARY_MAJOR_VERSION):0:0

libtrudp_la_LIBADD = $(TRUDP_LIBS)
