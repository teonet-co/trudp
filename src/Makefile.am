## Process this file with automake to produce Makefile.in
#

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(TRUDP_CFLAGS)

AM_CFLAGS = \
	 -g -Wall \
	 -std=gnu11 \
	 -fno-strict-aliasing \
	 -fPIC \
	 -DUSE_LIBEV \
	 -I../embedded/teoccl/src \
	 -fvisibility=hidden
	
#	 -std=gnu11 
#	 -std=gnu89
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

LIBS = ../embedded/teoccl/src/.libs/libteoccl.a

lib_LTLIBRARIES = libtrudp.la

libtrudp_la_SOURCES = \
	packet.c \
	packet_queue.c \
	trudp.c \
	trudp_receive_queue.c \
	trudp_send_queue.c \
	trudp_channel.c \
	trudp_utils.c \
	trudp_stat.c \
	trudp_ev.c \
	udp.c \
	utils.c \
	write_queue.c
	
libtrudpincludedir=$(includedir)
	
nobase_libtrudpinclude_HEADERS = \
	trudp.h

libtrudp_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBRARY_MAJOR_VERSION):0:0

libtrudp_la_LIBADD = $(TRUDP_LIBS)
