AM_CPPFLAGS = -I$(top_srcdir)/lib @OPENSSL_INCLUDES@
AM_LDFLAGS = -L$(top_srcdir)/lib @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ -lfuse

noinst_PROGRAMS = horus_util horusfs
horus_util_SOURCES = horus_util.c
horus_util_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
horusfs_SOURCES = horusfs.c
horusfs_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
