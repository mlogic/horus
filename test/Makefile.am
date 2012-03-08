TESTS = check_aes check_horus_key check_api
AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/aes
AM_LDFLAGS = -L$(top_srcdir)/lib

check_PROGRAMS = check_aes check_horus_key check_api
check_aes_SOURCES = check_aes.c \
    $(top_srcdir)/aes/gf128mul.c $(top_srcdir)/aes/xts.c \
    $(top_srcdir)/aes/aesni-intel_glue.c \
    $(top_srcdir)/aes/aesni-intel_asm.S

check_api_SOURCES = check_api.c
check_api_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
check_api_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@

check_horus_key_SOURCES = check_horus_key.c
check_horus_Key_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
check_horus_key_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@
