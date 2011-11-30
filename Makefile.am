
SUBDIRS = misc

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = horus_key
horus_key_SOURCES = horus_key.c
horus_key_LDADD = @OPENSSL_LIBS@

lib_LTLIBRARIES = libhorus.la
libhorus_la_SOURCES = sysio.c

clean-local: clean-local-dSYM
distclean-local: distclean-local-autom4te-cache
.PHONY: clean-local-dSYM distclean-local-autom4te-cache
clean-local-dSYM:
	-rm -rf *.dSYM
distclean-local-autom4te-cache:
	-rm -rf autom4te.cache
