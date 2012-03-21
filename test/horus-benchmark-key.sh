#!/bin/sh -x

filename=FILENAME
horus_file=~/bin/horus-file
horus_key=~/bin/horus-key

if [ ! -f $filename ]; then
  touch $filename;
fi

$horus_file FILENAME master-key "Horus Rocks!?"
$horus_file FILENAME kht-block-sizes 1G 256M 64M 16M 4M 1M 256K 64K 16K 4K

logfile=horus-benchmark-key.log
datafile=horus-benchmark-key.data
gpfile=horus-benchmark-key.gp

rm -f $logfile

for depth in `jot 10 1`; do
  ~/bin/horus-key FILENAME -b $depth >> $logfile
done

grep 'q/s' $logfile > $datafile
gnuplot $gpfile

blogfile=horus-benchmark-key-branch.log
bdatafile=horus-benchmark-key-branch.data
bgpfile=horus-benchmark-key-branch.gp

rm -f $blogfile

for branch in `jot 9 2`; do
  ~/bin/horus-key FILENAME -b 10 $branch >> $blogfile
done

grep 'q/s' $blogfile > $bdatafile
gnuplot $bgpfile


