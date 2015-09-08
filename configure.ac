#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libcamsrcjpegenc-sw], [0.1])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADER([config.h])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.

dnl use dlog --------------------------------------------------------------------------
AC_ARG_ENABLE(dlog, AC_HELP_STRING([--enable-dlog], [using dlog]),
[
 case "${enableval}" in
 	  yes) USE_DLOG=yes ;;
 	  no)  USE_DLOG=no ;;
          *)   AC_MSG_ERROR(bad value ${enableval} for --enable-dlog) ;;
 esac
],[USE_DLOG=yes])

if test "x$USE_DLOG" = "xyes"; then
        PKG_CHECK_MODULES(DLOG, dlog)
        AC_SUBST(DLOG_CFLAGS)
        AC_SUBST(DLOG_LIBS)
fi
AM_CONDITIONAL(USE_DLOG, test "x$USE_DLOG" = "xyes")
dnl end --------------------------------------------------------------------

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([malloc.h sys/mman.h sys/ioctl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostname gettimeofday memset uname])

PKG_CHECK_MODULES(GLIB, glib-2.0)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(CAMSRCJPEGENC, camsrcjpegenc)
AC_SUBST(CAMSRCJPEGENC_CFLAGS)
AC_SUBST(CAMSRCJPEGENC_LIBS)

PKG_CHECK_MODULES(MMUTIL_IMGP, mmutil-imgp)
AC_SUBST(MMUTIL_IMGP_CFLAGS)
AC_SUBST(MMUTIL_IMGP_LIBS)

LIBJPEG_LIBS="-ljpeg"
AC_SUBST(LIBJPEG_LIBS)

#AM_CONDITIONAL([ISPROTECTOR_VODA_SDK], [test "x$DISTRO" = "xvodafone-sdk" && test "x$MACHINE" = "xprotector"])

AC_CONFIG_FILES([Makefile
                 src/Makefile
])
AC_OUTPUT

