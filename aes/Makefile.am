AM_CPPFLAGS = -I$(top_srcdir)/aes @OPENSSL_INCLUDES@

lib_LTLIBRARIES = libaes.la
libaes_la_SOURCES = xts.c gf128mul.c \
    aes.h asm-alternative.h asm-bitops.h asm-inst.h \
    asm-linkage.h asm-swab.h b128ops.h bitops.h bitsperlong.h \
    byteorder-generic.h compiler.h crypto.h gf128mul.h linux-linkage.h \
    linux-types.h little_endian.h stringify.h swab.h xts.h

if AESNI
libaes_la_SOURCES += aesni-intel_glue.c aesni-intel_asm.S
else
libaes_la_SOURCES += aes.c
endif
