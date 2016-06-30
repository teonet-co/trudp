#                                               -*- Autoconf -*-
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
# END

AC_PREREQ([2.69])
AC_INIT([trudp], [0.2.0], [kirill@scherba.ru])
AC_DEFINE_SUBST(LIBRARY_MAJOR_VERSION, 1, [TR-UDP dynamic library version])
AC_DEFINE(COPYRIGHT, ["(c) Kirsoft Inc., 1996-2016"], [TR-UDP library copyright])
AC_CONFIG_SRCDIR([src/tr-udp.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])

AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime gethostbyname gettimeofday inet_ntoa memset select strdup])

dnl ***************************************************************************
dnl Debug info
dnl ***************************************************************************

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

dnl ***************************************************************************
dnl Check and set OS
dnl ***************************************************************************

AC_CANONICAL_HOST

case $host_os in
  *linux*)
    linux=true
    AC_DEFINE(HAVE_LINUX, 1, [Linux])
  ;;
  *freebsd*)
    bsd=true
    AC_DEFINE(HAVE_FREEBSD, 1, [FreeBSD])
  ;;
  *darwin*)
    bsd=true
    darwin=true
    AC_DEFINE(HAVE_DARWIN, 1, [Darwin (MacOS/X)])
  ;;
  *solaris*)
    solaris=true
    AC_DEFINE(HAVE_SOLARIS, 1, [Solaris/SunOS])
  ;;
  *openbsd*)
    bsd=true
    AC_DEFINE(HAVE_OPENBSD, 1, [OpenBSD])
  ;;
  *netbsd*)
    bsd=true
    AC_DEFINE(HAVE_NETBSD, 1, [NetBSD])
  ;;
  *dragonfly*)
    bsd=true
    AC_DEFINE(HAVE_DRAGONFLY, 1, [DragonFly])
  ;;
  *bsd*)
    bsd=true
    AC_MSG_WARN("Unknown BSD variant, ksnet might not compile or work!")
    AC_DEFINE(HAVE_BSD, 1, [Unknown BSD variant])
  ;;
  *cygwin*)
    cygwin=true
    AC_DEFINE(HAVE_CYGWIN, 1, [Cygwin])
  ;;
  *mingw*)
    mingw=true
    AC_DEFINE(HAVE_MINGW, 1, [MinGW])
    #LIBS="$LIBS -lws2_32 -lgdi32 -lcrypt32"
  ;;
  *)
    AC_MSG_ERROR("Unknown operating system.")
  ;;
esac

AC_ARG_ENABLE(uml,
  AS_HELP_STRING([--enable-uml], [enable support for User Mode Linux]),
  [ AS_IF([test "x$enable_uml" = "xyes"],
      [ AC_DEFINE(ENABLE_UML, 1, [Support for UML])
        uml=true
      ],
      [uml=false])
  ],
  [uml=false]
)

AC_ARG_ENABLE(vde,
  AS_HELP_STRING([--enable-vde], [enable support for Virtual Distributed Ethernet]),
  [ AS_IF([test "x$enable_vde" = "xyes"],
      [ AC_CHECK_HEADERS(libvdeplug_dyn.h, [], [AC_MSG_ERROR([VDE plug header files not found.]); break])
        AC_DEFINE(ENABLE_VDE, 1, [Support for VDE])
        vde=true
      ],
      [vde=false])
  ],
  [vde=false]
)

AC_ARG_ENABLE(tunemu,
  AS_HELP_STRING([--enable-tunemu], [enable support for the tunemu driver]),
  [ AS_IF([test "x$enable_tunemu" = "xyes"],
      [ AC_DEFINE(ENABLE_TUNEMU, 1, [Support for tunemu])
        tunemu=true
      ],
      [tunemu=false])
  ],
  [tunemu=false]
)

AC_ARG_WITH(windows2000,
  AS_HELP_STRING([--with-windows2000], [compile with support for Windows 2000. This disables support for tunneling over existing IPv6 networks.]),
  [ AS_IF([test "x$with_windows2000" = "xyes"],
      [AC_DEFINE(WITH_WINDOWS2000, 1, [Compile with support for Windows 2000])])
  ]
)

AM_CONDITIONAL(LINUX, test "$linux" = true)
AM_CONDITIONAL(BSD, test "$bsd" = true)
AM_CONDITIONAL(SOLARIS, test "$solaris" = true)
AM_CONDITIONAL(MINGW, test "$mingw" = true)
AM_CONDITIONAL(CYGWIN, test "$cygwin" = true)
AM_CONDITIONAL(UML, test "$uml" = true)
AM_CONDITIONAL(VDE, test "$vde" = true)
AM_CONDITIONAL(TUNEMU, test "$tunemu" = true)
AM_CONDITIONAL(DARWIN, test "$darwin" = true)

AM_CONDITIONAL(NOT_E_CONFUSE, true)
AM_CONDITIONAL(TEO_THREAD, false)
# if test $TEO_THREAD = "true"; then
# AC_DEFINE(TEO_THREAD, 1, [Teonet run in thread])
# fi

AC_CACHE_SAVE

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=teonet
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_GLIB_GNU_GETTEXT

dnl ***************************************************************************
dnl Documentation
dnl ***************************************************************************

# Checks for doxygen
AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, true, false)
AM_CONDITIONAL(HAVE_DOXYGEN,$HAVE_DOXYGEN)
if test $HAVE_DOXYGEN = "false"; then
    AC_MSG_WARN([*** doxygen not found, docs will not be built])
else

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(ON)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(TR-UDP, Doxyfile, docs)

fi

AC_CACHE_SAVE

LT_INIT

AC_CONFIG_FILES([
    Makefile
    po/Makefile.in
    src/libtrudp.pc
    src/Makefile
])
AC_OUTPUT
