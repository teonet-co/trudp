ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(TRUDP_CFLAGS)

AM_CFLAGS = \
	-g -Wall \
	-std=gnu11 \
	-I../libteoccl/include \
	-I../src

AM_CXX = clang++
AM_CXXFLAGS = \
	-g -Wall \
	-std=c++14 \
	-I../libteoccl/include \
	-I../src

## Debug info
if DEBUG
  AM_CFLAGS += -g3 -O0
  AM_CXXFLAGS += -g3 -O0
else
  AM_CFLAGS += -O2
  AM_CXXFLAGS += -O2
endif

AM_LDFLAGS = -L../src/.libs 

LIBS = ../src/.libs/libtrudp.a ../libteoccl/src/.libs/libteoccl.a -lcunit -lev

noinst_PROGRAMS = trudp_tst

trudp_tst_SOURCES = main_t.c packet_queue_t.c packet_t.c tr-udp_t.c

testsdir = $(datarootdir)/doc/@PACKAGE@/tests
tests_DATA = $(trudp_tst_SOURCES)

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(datarootdir)/doc/@PACKAGE@/tests