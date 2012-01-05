
AM_CPPFLAGS = -I$(top_srcdir)/lib
AM_LDFLAGS = -L$(top_srcdir)/lib

noinst_PROGRAMS = test-shell test-command-shell test-pthread
test_shell_SOURCES = test_shell.c
test_shell_LDADD = ../lib/libhorus.la
test_command_shell_SOURCES = test_command_shell.c
test_command_shell_LDADD = ../lib/libhorus.la
test_pthread_SOURCES = test_pthread.c
test_pthread_LDADD = @PTHREAD_LIBS@

bin_PROGRAMS = horus-key
horus_key_SOURCES = horus_key.c
horus_key_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@

sbin_PROGRAMS = horus-kds
horus_kds_SOURCES = horus_kds.c
horus_kds_LDADD = ../lib/libhorus.la @OPENSSL_LIBS@
