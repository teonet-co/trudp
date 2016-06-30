## Process this file with automake to produce Makefile.in
#

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(TRUDP_CFLAGS)

AM_CFLAGS = \
	 -Wall \
	 -g
	
#	 -std=c11 
#	 -I$(HOME)/Projects/teonet/src/ 
#	 -I$(HOME)/Projects/teonet/embedded/libpbl/src 
#	 -I$(HOME)/Projects/teonet/embedded/teocli/libteol0 
#	 -I/usr/include/teonet

AM_LDFLAGS = 

AM_CXX = clang++
AM_CXXFLAGS = \
         -g -Wall

#	 -std=c++11 
#	 -I/opt/localionclude 
#	 -I$(HOME)/Projects/teonet/src/

## Debug info
if DEBUG
  AM_CFLAGS += -g3 -O0
  AM_CXXFLAGS += -g3 -O0
else
  AM_CFLAGS += -O2
  AM_CXXFLAGS += -O2
endif

LIBS =

lib_LTLIBRARIES = libtrudp.la

libtrudp_la_SOURCES = \
	tr-udp.c

libtrudp_la_LDFLAGS =

libtrudp_la_LIBADD = $(TRUDP_LIBS)
