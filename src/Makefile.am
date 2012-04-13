
AM_CPPFLAGS = -I. -I$(top_srcdir)/lib -I$(top_srcdir)/aes  -I$(top_srcdir)/test @OPENSSL_INCLUDES@
AM_LDFLAGS = -L$(top_srcdir)/lib @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@

bin_PROGRAMS = horus-file horus-key kds_client
sbin_PROGRAMS = horus-kds kds_server kds_server_thpool

horus_file_SOURCES = horus_file.c
horus_file_LDADD = ../lib/libhorus.la

horus_key_SOURCES = horus_key.c
horus_key_LDADD = ../lib/libhorus.la

horus_kds_SOURCES = horus_kds.c
# horus_kds_CFLAGS = @PTHREAD_CFLAGS@
horus_kds_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@ @PTHREAD_LIBS@

kds_server_SOURCES = kds_server.c
kds_server_LDADD = ../lib/libhorus.la @PTHREAD_LIBS@


kds_server_thpool_SOURCES = thpool.c kds_server_thpool.c
kds_server_thpool_LDADD = ../lib/libhorus.la @PTHREAD_LIBS@

kds_client_SOURCES = kds_client.c
kds_client_LDADD = ../lib/libhorus.la @PTHREAD_LIBS@


noinst_PROGRAMS = test-shell test-command-shell test-pthread test-openssl
test_shell_SOURCES = test_shell.c
test_shell_LDADD = ../lib/libhorus.la
test_command_shell_SOURCES = test_command_shell.c
test_command_shell_LDADD = ../lib/libhorus.la
test_pthread_SOURCES = test_pthread.c
# test_pthread_CFLAGS = @PTHREAD_CFLAGS@
test_pthread_LDADD = @PTHREAD_LIBS@
test_openssl_SOURCES = test_openssl.c
test_openssl_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
