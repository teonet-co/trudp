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

LIBS =../src/.libs/libtrudp.a -lev 
noinst_PROGRAMS = trudp_tst

trudp_tst_SOURCES = main_t.c

testsdir = $(datarootdir)/doc/@PACKAGE@/tests
tests_DATA = $(trudp_tst_SOURCES)

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(datarootdir)/doc/@PACKAGE@/tests
