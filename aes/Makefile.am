AM_CPPFLAGS = -I$(top_srcdir)/aes @OPENSSL_INCLUDES@

lib_LTLIBRARIES = libaes.la
libaes_la_SOURCES = xts.c gf128mul.c 

if AESNI
libaes_la_SOURCES += aesni-intel_glue.c aesni-intel_asm.S
else
libaes_la_SOURCES += aes.c
endif
