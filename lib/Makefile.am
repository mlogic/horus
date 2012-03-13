AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/aes @OPENSSL_INCLUDES@

lib_LTLIBRARIES = libhorus.la
libhorus_la_SOURCES = \
    log.c log.h thread.c thread.h network.c network.h \
    horus_key.c horus_key.h \
    horus.c horus.h \
    vectorx.c vectorx.h vectorx_sort.c vectorx_sort.h \
    shell.c shell.h shell_history.c shell_history.h \
    command.c command.h command_shell.c command_shell.h \
    terminal.c terminal.h openssl.c openssl.h \
    horus_attr.h horus_attr.c uthash.h $(top_srcdir)/aes/xts.c \
    $(top_srcdir)/aes/gf128mul.c 
if INTEL_AESNI
libhorus_la_SOURCES += \
    $(top_srcdir)/aes/aesni-intel_glue.c \
    $(top_srcdir)/aes/aesni-intel_asm.S
else
libhorus_la_SOURCES += $(top_srcdir)/aes/aes.c
endif
