ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(TRUDP_CFLAGS)

AM_CFLAGS = \
	-g -Wall \
	-std=gnu11 \
	-fno-strict-aliasing \
	-DUSE_LIBEV \
	-I../src \
	-I../embedded/teoccl/include

AM_CXX = clang++
AM_CXXFLAGS = \
	-g -Wall \
	-std=c++14 \
	-I../embedded/teoccl/include \
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

LIBS = -ltrudp -lev
trudp2p_LDFLAGS = $(AM_LDFLAGS) -pthread
#trudpcat_ev_LDFLAGS = $(AM_LDFLAGS) -lev

noinst_PROGRAMS = trudpcat_ev trudp2p trudpcat_ev_wq

trudp2p_SOURCES = trudp2p.c trudp_pth.c read_queue.c 
trudpcat_ev_SOURCES = trudpcat_ev.c 
trudpcat_ev_wq_SOURCES = trudpcat_ev_wq.c 

#snake.c

examplesdir = $(datarootdir)/doc/@PACKAGE@/examples
examples_DATA = trudpcat_ev.c snake.c

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(datarootdir)/doc/@PACKAGE@/examples
