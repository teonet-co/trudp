ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(TRUDP_CFLAGS)

AM_CFLAGS = \
	 -g -Wall \
	 -fno-strict-aliasing \
	 -DUSE_LIBEV \
	 -I../src

AM_CXX = clang++
AM_CXXFLAGS = \
         -g -Wall \
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

noinst_PROGRAMS = trudpcat_ev
trudpcat_ev_SOURCES = trudpcat_ev.c snake.c

examplesdir = $(datarootdir)/doc/@PACKAGE@/examples
examples_DATA = trudpcat_ev.c snake.c

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(datarootdir)/doc/@PACKAGE@/examples