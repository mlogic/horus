#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libhorus, 0.0.1, cthulhu@soe.ucsc.edu)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([horus_key.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT($HOME)

CFLAGS="$CFLAGS -g -O2 -Wall"

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

# Libtool
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

AC_CONFIG_FILES([Makefile misc/libhorus-env.sh])
AC_OUTPUT
