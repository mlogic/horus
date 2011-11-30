
sysconf_DATA = libhorus-env.sh libhorus-env.csh
CLEANFILES = libhorus-env.sh libhorus-env.csh
EXTRA_DIST = libhorus-env.sh.in libhorus-env.csh.in

do_subst = sed -e 's,[@]libdir[@],$(libdir),g'

libhorus-env.sh: libhorus-env.sh.in Makefile
	$(do_subst) < libhorus-env.sh.in > libhorus-env.sh

libhorus-env.csh: libhorus-env.csh.in Makefile
	$(do_subst) < libhorus-env.csh.in > libhorus-env.csh

