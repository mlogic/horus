TESTS = test_aes check_horus_key
AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/aes
AM_LDFLAGS = -L$(top_srcdir)/lib

check_PROGRAMS = test_aes check_horus_key
test_aes_SOURCES = test_aes.c $(top_srcdir)/aes/aes.c $(top_srcdir)/aes/xts.c $(top_srcdir)/aes/gf128mul.c

# test_api_SOURCES = test_api.c
# test_api_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
# test_api_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@

check_horus_key_SOURCES = check_horus_key.c
check_horus_Key_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
check_horus_key_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@
