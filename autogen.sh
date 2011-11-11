#!/bin/sh

rm -f config.cache
aclocal -I m4
autoheader
automake --add-missing --copy --warnings=all --include-deps --foreign
autoconf

