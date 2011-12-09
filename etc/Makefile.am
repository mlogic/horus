
sysconf_DATA = \
	libhorus-start.sh libhorus-start.csh \
	libhorus-stop.sh libhorus-stop.csh
EXTRA_DIST = \
	libhorus-start.sh.in libhorus-start.csh.in \
	libhorus-stop.sh.in libhorus-stop.csh.in

CLEANFILES = $(sysconf_DATA)

do_subst = sed -e 's,[@]libdir[@],$(libdir),g'

libhorus-start.sh: libhorus-start.sh.in Makefile
	$(do_subst) < $(srcdir)/$@.in > $@

libhorus-stop.sh: libhorus-stop.sh.in Makefile
	$(do_subst) < $(srcdir)/$@.in > $@

libhorus-start.csh: libhorus-start.csh.in Makefile
	$(do_subst) < $(srcdir)/$@.in > $@

libhorus-stop.csh: libhorus-stop.csh.in Makefile
	$(do_subst) < $(srcdir)/$@.in > $@

