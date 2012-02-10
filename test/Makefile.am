TESTS = test_aes
AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/aes
AM_LDFLAGS = -L$(top_srcdir)/lib

check_PROGRAMS = test_aes
test_aes_SOURCES = test_aes.c $(top_srcdir)/aes/aes.c $(top_srcdir)/aes/xts.c $(top_srcdir)/aes/gf128mul.c
