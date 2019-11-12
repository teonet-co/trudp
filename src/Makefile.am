lib_LTLIBRARIES = libtrudp.la

AM_CXXFLAGS = \
    -std=c++17 \
    -fPIC \
    -Wall \
    -Wpointer-arith \
    -I../libs/teobase/src \
    -I../libs/teobase/include \
    -I../libs/teoccl/src \
    -I../libs/teoccl/include


AM_CFLAGS = -fno-strict-aliasing \
	-fPIC \
	-DUSE_LIBEV \
	-fvisibility=hidden \
	-I../libs/teobase/src \
    	-I../libs/teobase/include \
    	-I../libs/teoccl/src \
    	-I../libs/teoccl/include \
	-Wpointer-arith


libtrudpincludedir = \
    $(includedir)/libtrudp \
    $(includedir)/libs/teobase/src \
    $(includedir)/libs/teobase/include \
    $(includedir)/libs/teoccl/src \
    $(includedir)/libs/teoccl/include


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
	write_queue.c \
    ../libs/teobase/src/teobase/logging.c \
    ../libs/teobase/src/teobase/socket.c \
    ../libs/teobase/src/teobase/time.c \
    ../libs/teoccl/src/teoccl/array_list.c \
    ../libs/teoccl/src/teoccl/hash.c \
    ../libs/teoccl/src/teoccl/map.c \
    ../libs/teoccl/src/teoccl/queue.c




libtrudpinclude_HEADERS = 

libteoccl_la_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/libs/teoccl/include
libtrudp_la_LDFLAGS = -version-info $(LIBRARY_CURRENT):$(LIBRARY_REVISION):$(LIBRARY_AGE)

uninstall-hook:
	-rmdir $(libtrudpincludedir)
