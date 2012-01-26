#!/bin/sh -x

sh -x make-pics.sh
platex test.tex
dvipdfm test

