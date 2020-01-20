lib_LTLIBRARIES = libtrudp.la


AM_CFLAGS = -fno-strict-aliasing \
	-fPIC \
	-DUSE_LIBEV \
	-fvisibility=hidden \
	-I../libs/teobase/include \
	-I../libs/teoccl/include \
	-Wpointer-arith \
	# end of AM_CFLAGS


libtrudpincludedir = $(includedir)/libtrudp

libtrudp_la_SOURCES = \
    packet.c \
    packet_queue.c \
    trudp.c \
    trudp_options.c \
    trudp_receive_queue.c \
    trudp_send_queue.c \
    trudp_channel.c \
    trudp_utils.c \
    trudp_stat.c \
    trudp_ev.c \
    udp.c \
    write_queue.c \
    \
    ../libs/teobase/src/teobase/logging.c \
    ../libs/teobase/src/teobase/mutex.c \
    ../libs/teobase/src/teobase/socket.c \
    ../libs/teobase/src/teobase/time.c \
    \
    ../libs/teoccl/src/teoccl/array_list.c \
    ../libs/teoccl/src/teoccl/hash.c \
    ../libs/teoccl/src/teoccl/map.c \
    ../libs/teoccl/src/teoccl/memory.c \
    ../libs/teoccl/src/teoccl/queue.c \
    # ../libs/teoccl/src/teoccl/deque.c \
    # ../libs/teoccl/src/teoccl/linked_list.c \
    # ../libs/teoccl/src/teoccl/list.c \
    # ../libs/teoccl/src/teoccl/que.c \
    # ../libs/teoccl/src/teoccl/stack.c \
    # end of libtrudp_la_SOURCES
#  ^^^^ Some containers are not production ready yet ^^^

teobasedir = $(pkgincludedir)/teobase
teoccldir = $(pkgincludedir)/teoccl

#pkginclude_HEADERS =

nobase_libtrudpinclude_HEADERS = \
	packet.h \
	packet_queue.h \
	trudp.h \
	trudp_options.h \
	trudp_receive_queue.h \
	trudp_send_queue.h \
	trudp_channel.h \
	trudp_utils.h \
	trudp_stat.h \
	trudp_ev.h \
	udp.h \
	write_queue.h \
	# end of nobase_libtrudpinclude_HEADERS

teobase_HEADERS = \
	$(top_srcdir)/libs/teobase/include/teobase/api.h \
	$(top_srcdir)/libs/teobase/include/teobase/logging.h \
	$(top_srcdir)/libs/teobase/include/teobase/mutex.h \
	$(top_srcdir)/libs/teobase/include/teobase/platform.h \
	$(top_srcdir)/libs/teobase/include/teobase/socket.h \
	$(top_srcdir)/libs/teobase/include/teobase/time.h \
	$(top_srcdir)/libs/teobase/include/teobase/windows.h \
	# end of teobase_HEADERS
teoccl_HEADERS = \
	$(top_srcdir)/libs/teoccl/include/teoccl/array_list.h \
	$(top_srcdir)/libs/teoccl/include/teoccl/hash.h \
	$(top_srcdir)/libs/teoccl/include/teoccl/map.h \
	$(top_srcdir)/libs/teoccl/include/teoccl/memory.h \
	$(top_srcdir)/libs/teoccl/include/teoccl/queue.h \
	# end of teoccl_HEADERS


libteoccl_la_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/libs/teoccl/include
libtrudp_la_LDFLAGS = -version-info $(LIBRARY_CURRENT):$(LIBRARY_REVISION):$(LIBRARY_AGE)

uninstall-hook:
	-rmdir $(teobasedir)
	-rmdir $(teoccldir)
	-rmdir $(libtrudpincludedir)
