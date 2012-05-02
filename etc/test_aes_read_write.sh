#!/bin/sh -x

dir=/tmp
file=testdata

rm $dir/$file
touch $dir/$file

~/bin/horus-file $dir/$file master-key "this is a test"
~/bin/horus-file $dir/$file kht-block-sizes 4096G 512G 64G 8G 1G 128M 16M 2M 256K 32K 4K
~/bin/horus-file $dir/$file client add 127.0.0.1 0 4G


dd if=/dev/random of=$dir/$file bs=4096 count=10
hexdump $dir/$file > $dir/$file.txt

~/bin/horus-read-write -l 40K -v $dir/$file -u -r -i $dir/$file -e -w -o $dir/$file.enc 2>&1 | tee $dir/$file.enc.log

~/bin/horus-read-write -l 40K -v $dir/$file -u -r -g -i $dir/$file.enc -w -o $dir/$file.dec 2>&1 | tee $dir/$file.dec.log

hexdump $dir/$file.dec > $dir/$file.dec.txt

diff -u $dir/$file.txt $dir/$file.dec.txt > $dir/$file.diff

