
AM_CPPFLAGS = -I$(top_srcdir)/lib
AM_LDFLAGS = -L$(top_srcdir)/lib

bin_PROGRAMS = horus-key
horus_key_SOURCES = horus_key.c
horus_key_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@

sbin_PROGRAMS = horus-kds
horus_kds_SOURCES = horus_kds.c
horus_kds_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
