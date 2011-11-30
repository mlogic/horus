
SUBDIRS = misc

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = horus_key
horus_key_SOURCES = horus_key.c
horus_key_LDADD = @OPENSSL_LIBS@

lib_LTLIBRARIES = libhorus.la
libhorus_la_SOURCES = sysio.c

clean-local: clean-local-dSYM
.PHONY: clean-local-dSYM

clean-local-dSYM:
	-rm -rf *.dSYM

