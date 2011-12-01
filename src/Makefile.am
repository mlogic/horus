
AM_CPPFLAGS = -I$(top_srcdir)/lib
AM_LDFLAGS = -L$(top_srcdir)/lib

bin_PROGRAMS = horus_key horus_key2
horus_key_SOURCES = horus_key.c
horus_key_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
horus_key2_SOURCES = horus_key2.c
horus_key2_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@

