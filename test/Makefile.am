TESTS = check_aes # check_horus_key check_api
AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/aes -I$(top_srcdir)/test
AM_LDFLAGS = -L$(top_srcdir)/lib

if LIBCHECK

check_PROGRAMS = check_aes 
check_aes_SOURCES = check_aes.c \
    $(top_srcdir)/aes/gf128mul.c $(top_srcdir)/aes/xts.c
if AESNI
check_aes_SOURCES += \
    $(top_srcdir)/aes/aesni-intel_glue.c \
    $(top_srcdir)/aes/aesni-intel_asm.S
else
check_aes_SOURCES += $(top_srcdir)/aes/aes.c
endif


# check_api_SOURCES = check_api.c
# check_api_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
# check_api_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@

# check_horus_key_SOURCES = check_horus_key.c
# check_horus_key_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
# check_horus_key_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@

# benchmark_key_utils_SOURCES = benchmark_key_utils.c benchmark.h
# benchmark_key_utils_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
# benchmark_key_utils_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@
# if APPLE
# benchmark_key_utils_LDFLAGS = -framework CoreServices
# else
# benchmark_key_utils_LDADD += -lrt
# endif

# benchmark_read_write_SOURCES = benchmark_read_write.c benchmark.h
# benchmark_read_write_CFLAGS = @CHECK_CFLAGS@ @OPENSSL_INCLUDES@
# benchmark_read_write_LDADD = @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@ $(top_builddir)/lib/libhorus.la @CHECK_LIBS@
# if APPLE
# benchmark_read_write_LDFLAGS = -framework CoreServices
# else
# benchmark_read_write_LDADD += -lrt
# endif

endif
