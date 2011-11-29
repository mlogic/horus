
ACLOCAL_AMFLAGS = -I m4

# LIBTOOL_DEPS = @LIBTOOL_DEPS@
# libtool: $(LIBTOOL_DEPS)
# 	$(SHELL) ./config.status libtool

bin_PROGRAMS = horus_key
horus_key_SOURCES = horus_key.c
horus_key_LDADD = @OPENSSL_LIBS@

lib_LTLIBRARIES = libhorus.la
libhorus_la_SOURCES = sysio.c

bin_SCRIPTS = misc/libhorus-env.sh

