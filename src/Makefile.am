
AM_CPPFLAGS = -I. -I$(top_srcdir)/lib -I$(top_srcdir)/aes  -I$(top_srcdir)/test @OPENSSL_INCLUDES@
AM_LDFLAGS = -L$(top_srcdir)/lib @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@

bin_PROGRAMS = horus-file horus-key kds_client
sbin_PROGRAMS = kds_server

horus_file_SOURCES = horus_file.c
horus_file_LDADD = ../lib/libhorus.la

horus_key_SOURCES = horus_key.c
horus_key_LDADD = ../lib/libhorus.la

kds_server_SOURCES = kds_server.c
kds_server_LDADD = ../lib/libhorus.la @PTHREAD_LIBS@

kds_client_SOURCES = kds_client.c
kds_client_LDADD = ../lib/libhorus.la @PTHREAD_LIBS@

