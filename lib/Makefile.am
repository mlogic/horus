AM_CPPFLAGS = -I$(top_srcdir)/aes  -I$(top_srcdir)/lib @OPENSSL_INCLUDES@

AM_LDFLAGS = ../aes/libaes.la
lib_LTLIBRARIES = libhorus.la
libhorus_la_SOURCES = \
    log.c log.h network.c network.h \
    horus_key.c horus_key.h horus.c horus.h \
    horus_attr.h horus_attr.c horus_stats.c horus_stats.h \
    kds_protocol.c kds_protocol.h horusio.h horusio.c \
    horus_crypt.c horus_crypt.h

