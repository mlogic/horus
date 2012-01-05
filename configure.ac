#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(horus, 0.0.2, horus@soe.ucsc.edu)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([lib/horus.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT($HOME)

CFLAGS="$CFLAGS -g -O0 -Wall"

AC_ARG_ENABLE([horus],
  [AS_HELP_STRING([--enable-horus],
                  [enable horus functions [default=yes]])])
AC_ARG_ENABLE([log-syslog],
  [AS_HELP_STRING([--enable-log-syslog],
                  [enable logging to syslog [default=yes]])])
AC_ARG_ENABLE([log-stderr],
  [AS_HELP_STRING([--enable-log-stderr],
                  [enable logging to stderr])])

if test "$enable_horus" != "no"; then
  AC_DEFINE(ENABLE_HORUS,,Enable Horus)
fi
if test "$enable_log_syslog" != "no"; then
  AC_DEFINE(ENABLE_SYSLOG,,Logging to syslog)
fi
if test "$enable_log_stderr" = "yes"; then
  AC_DEFINE(ENABLE_STDERR,,Logging to stderr)
fi

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AX_CHECK_OPENSSL([AC_DEFINE([HAVE_OPENSSL],1,[OpenSSL support.])])

AX_PTHREAD([AC_DEFINE([HAVE_PTHREAD],1,[POSIX thread support.])])

# Libtool
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile etc/Makefile])
AC_OUTPUT
