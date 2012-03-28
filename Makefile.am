
SUBDIRS = lib src test

# AM_CPPFLAGS = -I$(top_srcdir)/lib @PTHREAD_CFLAGS@

ACLOCAL_AMFLAGS = -I m4

clean-local: clean-local-dSYM
distclean-local: distclean-local-autom4te-cache
.PHONY: clean-local-dSYM distclean-local-autom4te-cache
clean-local-dSYM:
	-rm -rf *.dSYM
distclean-local-autom4te-cache:
	-rm -rf autom4te.cache

