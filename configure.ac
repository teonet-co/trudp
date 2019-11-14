#                                        \       -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# TR-UDP: Teonet Real time communications over UDP protocol
#

# SUBROUTINE
#
# AC_DEFINE_SUBST(NAME, VALUE, DESCRIPTION)
# -----------------------------------------
AC_DEFUN([AC_DEFINE_SUBST], [
AC_DEFINE([$1], [$2], [$3])
AC_SUBST([$1], ['$2'])
])

m4_define([pkg_major], [0]) dnl
m4_define([pkg_minor], [4]) dnl
m4_define([pkg_patch], [0]) dnl
m4_define([pkg_version], [pkg_major[.]pkg_minor[.]pkg_patch]) dnl



AC_PREREQ([2.69])
AC_INIT([libtrudp], [pkg_version], [kirill@scherba.ru, mpano91@gmail.com])

AC_SUBST([DOLLAR_SIGN],[$])

AC_DEFINE([PACKAGE_DESCRIPTION], ["TR-UDP library: Teonet Real time communications over UDP protocol"], [Application description])
#AC_DEFINE([PACKAGE_DEPENDENCIES], ["libev"], [Application dependencies])
AC_DEFINE([PACKAGE_DEPENDENCIES], [""], [Application dependencies])
AC_DEFINE([LICENSES], ["[[\"MIT\",\"LGPL-3.0\"]]"], [Application package licenses])
AC_DEFINE([VCS_URL], ["https://github.com/teonet-co/trudp.git"], [Git URL])
AC_DEFINE(COPYRIGHT, ["(c) Kirsoft Inc., 1996-2019"], [TR-UDP library copyright])

PKG_INSTALLDIR




AC_DEFINE_SUBST(LIBRARY_CURRENT,  1, [tr-udp dynamic library version])
AC_DEFINE_SUBST(LIBRARY_REVISION, 4, [tr-udp dynamic library version])
AC_DEFINE_SUBST(LIBRARY_AGE,      0, [tr-udp dynamic library version])

AC_DEFINE_SUBST(LIBRARY_MAJOR_VERSION, [pkg_major], [pack ver])
AC_DEFINE_SUBST(LIBRARY_MINOR_VERSION, [pkg_minor], [pack ver])
AC_DEFINE_SUBST(LIBRARY_PATCH_VERSION, [pkg_patch], [pack ver])


AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/trudp.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])


AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES([yes])


AC_PROG_CC
AC_PROG_CXX
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])


LT_PREREQ([2.4])
LT_INIT

# Checks for doxygen
AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, true, false)
AM_CONDITIONAL(HAVE_DOXYGEN,$HAVE_DOXYGEN)
if test $HAVE_DOXYGEN = "false"; then
    AC_MSG_WARN([*** doxygen not found, docs will not be built])
else

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(Trudp, Doxyfile, docs)

fi

AC_CONFIG_FILES([
    Makefile
    src/libtrudp.pc
    src/Makefile
    tests/Makefile
    examples/Makefile
])


#AC_CONFIG_SUBDIRS([
#    libs/teobase
#    libs/teoccl
#])

AC_OUTPUT
