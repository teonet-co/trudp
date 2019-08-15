## Process this file with automake to produce Makefile.in
#

ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = \
	-g -Wall \
	-std=gnu11 \
	-fno-strict-aliasing \
	-fPIC \
	-DUSE_LIBEV \
	-I../libs/teobase/include \
	-I../libs/teobase/libs/teoccl/include \
	-fvisibility=hidden \
	-Wpointer-arith

AM_CXX = clang++
AM_CXXFLAGS = \
	-g -Wall \
	-std=c++14 \
	-fPIC \
	-I../libs/teobase/include \
	-I../libs/teobase/libs/teoccl/include \
	-I$(HOME)/Projects/teonet/embedded/libtrudp/src \
	-I/usr/include/libtrudp

## Debug info
if DEBUG
  AM_CFLAGS += -g3 -O0
  AM_CXXFLAGS += -g3 -O0
else
  AM_CFLAGS += -O2
  AM_CXXFLAGS += -O2
endif

AM_LDFLAGS = 

LIBS = \
    ../libs/teobase/src/.libs/libteobase.a \
    ../libs/teobase/libs/teoccl/src/.libs/libteoccl.a

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
	write_queue.c

libtrudpincludedir=$(includedir)/libtrudp

nobase_libtrudpinclude_HEADERS = 

libtrudp_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBRARY_MAJOR_VERSION):0:0

libtrudp_la_LIBADD = $(TRUDP_LIBS)
