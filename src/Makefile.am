lib_LTLIBRARIES = libtrudp.la

AM_CFLAGS = -fno-strict-aliasing \
	-fPIC \
	-DUSE_LIBEV \
	-fvisibility=hidden \
	-I../libs/teoccl/include \
	-I../libs/teobase/include \
	-Wpointer-arith


LIBS = ../libs/teoccl/src/.libs/libteoccl.a

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

libtrudpinclude_HEADERS = 

libteoccl_la_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/libs/teoccl/include
libtrudp_la_LDFLAGS = -version-info $(LIBRARY_CURRENT):$(LIBRARY_REVISION):$(LIBRARY_AGE)

uninstall-hook:
	-rmdir $(libtrudpincludedir)
