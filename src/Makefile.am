
AM_CPPFLAGS = -I$(top_srcdir)/lib
AM_LDFLAGS = -L$(top_srcdir)/lib

bin_PROGRAMS = horus_key
horus_key_SOURCES = horus_key.c
horus_key_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@

sbin_PROGRAMS = horus_key_server
horus_key_server_SOURCES = horus_key_server.c
horus_key_server_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
