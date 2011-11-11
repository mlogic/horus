#!/bin/sh

rm -f config.cache
aclocal -I m4
autoheader
automake -a --warnings=all --include-deps --foreign
autoconf

