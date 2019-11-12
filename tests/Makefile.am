AM_CFLAGS = \
	-g -Wall \
	-std=gnu11 \
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

LIBS =../src/.libs/libtrudp.a -lcunit -lev 
noinst_PROGRAMS = trudp_tst

trudp_tst_SOURCES = main_t.c packet_queue_t.c packet_t.c tr-udp_t.c

testsdir = $(datarootdir)/doc/@PACKAGE@/tests
tests_DATA = $(trudp_tst_SOURCES)

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(datarootdir)/doc/@PACKAGE@/tests
