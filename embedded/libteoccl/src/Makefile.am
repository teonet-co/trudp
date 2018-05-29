## Process this file with automake to produce Makefile.in
#

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(TEOCCL_CFLAGS)

AM_CFLAGS = \
	 -g -Wall \
	 -std=gnu11 \
	 -fno-strict-aliasing \
	 -fPIC

#	 -fvisibility=hidden
#	 -I$(HOME)/Projects/teonet/src/ 

AM_CXX = clang++
AM_CXXFLAGS = \
	 -g -Wall \
         -std=c++17 \
	 -fPIC
	
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

lib_LTLIBRARIES = libteoccl.la

libteoccl_la_SOURCES = \
	queue.c \
	hash.c \
	map.c
	
libteocclincludedir=$(includedir)
	
nobase_libteocclinclude_HEADERS = \
	queue.h

libteoccl_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBRARY_MAJOR_VERSION):0:0

libteoccl_la_LIBADD = $(TEOCCL_LIBS)
