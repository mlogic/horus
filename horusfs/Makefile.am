AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/aes -I$(top_srcdir)/src  @OPENSSL_INCLUDES@
AM_LDFLAGS = -L$(top_srcdir)/lib @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ -lfuse

if FUSE
noinst_PROGRAMS = horus_util horusfs
horus_util_SOURCES = horus_util.c
horus_util_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
horusfs_SOURCES = horusfs.c $(top_srcdir)/src/kds_rpc_clnt.c $(top_srcdir)/src/kds_rpc_xdr.c
horusfs_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
endif
