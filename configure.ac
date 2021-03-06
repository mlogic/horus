#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(horus, 0.0.3, yanli@ascar.io)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([lib/horus.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT($HOME)

# must explicitly reference lpthread when compiling on Fedora
# see https://fedoraproject.org/wiki/UnderstandingDSOLinkChange
PTHREAD_LIBS="-lpthread"

AC_ARG_ENABLE([debug],
     [  --enable-debug    Turn on debugging],
     [case "${enableval}" in
       yes) debug=true ;;
       no)  debug=false ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
     esac],[debug=false])
CFLAGS="$CFLAGS -Wall"
if test x$debug = xtrue; then
   CFLAGS="$CFLAGS -g -O0"
else
   CFLAGS="$CFLAGS -O2"
fi

AC_CANONICAL_HOST
case $host in
    *-apple-*)
      apple=true
      LDFLAGS="$LDFLAGS -framework CoreServices"
      ;;
    *-linux-*)
      LDFLAGS="$LDFLAGS -lrt"
      apple=false
      ;;
esac
AM_CONDITIONAL([APPLE], [test x$apple = xtrue])

AC_ARG_ENABLE([aesni],
     [  --enable-aesni    Turn on Intel AESNI],
     [case "${enableval}" in
       yes) intel_aesni=true ;;
       no)  intel_aesni=false ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-intel-aesni]) ;;
     esac],[aesni=false])
AM_CONDITIONAL([AESNI], [test x$aesni = xtrue])

AC_ARG_ENABLE([fuse],
     [  --enable-fuse    Build FUSE support],
     [case "${enableval}" in
       yes) fuse=true ;;
       no)  fuse=false ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-fuse]) ;;
     esac],[fuse=false])
AM_CONDITIONAL([FUSE], [test x$fuse = xtrue])


# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AS

# Checks for libraries.
PKG_CHECK_MODULES([CHECK], [check >= 0.9.8],[libcheck=true])
AM_CONDITIONAL([LIBCHECK], [test x$libcheck = xtrue])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AX_CHECK_OPENSSL([AC_DEFINE([HAVE_OPENSSL],1,[OpenSSL support.])],AC_MSG_ERROR([Missing OpenSSL]))

AX_PTHREAD([AC_DEFINE([HAVE_PTHREAD],1,[POSIX thread support.])],AC_MSG_ERROR([Missing pthread]))


# Libtool
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile aes/Makefile test/Makefile])
AC_OUTPUT
