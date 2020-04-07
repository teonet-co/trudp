AM_CFLAGS = \
	-g -Wall \
	-fno-strict-aliasing \
	-DUSE_LIBEV \
	-I../libs/teobase/include \
	-I../libs/teoccl/include \
	-I../src

AM_CXX = clang++
AM_CXXFLAGS = \
	-g -Wall \
	-std=c++14 \
	-I../libs/teobase/include \
	-I../libs/teoccl/include \
	-I../src

AM_LDFLAGS = -L../src/.libs

LIBS =../src/.libs/libtrudp.a -lev

trudp2p_LDFLAGS = $(AM_LDFLAGS) -pthread

noinst_PROGRAMS = trudpcat_ev trudp2p trudpcat_ev_wq

trudp2p_SOURCES = trudp2p.c trudp_pth.c read_queue.c 
trudpcat_ev_SOURCES = trudpcat_ev.c 
trudpcat_ev_wq_SOURCES = trudpcat_ev_wq.c 

examplesdir = $(datarootdir)/doc/@PACKAGE@/examples
examples_DATA = trudpcat_ev.c snake.c

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(datarootdir)/doc/@PACKAGE@/examples
