
AM_CPPFLAGS = -I$(top_srcdir)/lib @OPENSSL_INCLUDES@

lib_LTLIBRARIES = libhorus.la
libhorus_la_SOURCES = \
    log.c log.h thread.c thread.h network.c network.h \
    sysio.c horus_key.c horus_key.h \
    horus_crypt.c horus_crypt.h horus.c horus.h \
    vectorx.c vectorx.h vectorx_sort.c vectorx_sort.h \
    shell.c shell.h shell_history.c shell_history.h \
    command.c command.h command_shell.c command_shell.h \
    terminal.c terminal.h openssl.c openssl.h

