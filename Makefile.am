include aminclude.am
if DX_COND_html

trudpdatarootdocdir = $(datarootdir)/doc/@PACKAGE@
doxydocdir = $(datarootdir)/doc/@PACKAGE@/html
doxydoc_DATA = @DOLLAR_SIGN@(wildcard docs/html/*.*)

doxydoc_searchdir = $(datarootdir)/doc/@PACKAGE@/html/search
doxydoc_search_DATA = @DOLLAR_SIGN@(wildcard docs/html/search/*.*)

DX_CLEANFILES = -rf @DX_DOCDIR@

endif


ACLOCAL_AMFLAGS = -I m4
SUBDIRS = libs/teobase libs/teoccl src tests #examples
pkgconfig_DATA = src/libtrudp.pc


trudpdocdir = ${prefix}/doc/@PACKAGE@
trudpdoc_DATA =	ChangeLog

EXTRA_DIST = $(trudpdoc_DATA) \
	$(INTLTOOL_FILES)

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(trudpdocdir)
	
test:	tests/trudp_tst
	tests/trudp_tst
