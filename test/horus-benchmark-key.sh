#!/bin/sh -x

logfile=horus-benchmark-key.log
datafile=horus-benchmark-key.data

gpfile=horus-benchmark-key.gp
filename=FILENAME
horus_file=~/bin/horus-file
horus_key=~/bin/horus-key

rm -f $logfile

if [ ! -f $filename ]; then
  touch $filename;
fi

$horus_file FILENAME master-key "Horus Rocks!?"
$horus_file FILENAME kht-block-sizes 1G 256M 64M 4M 1M 256K 64K 16K 4K

for depth in `jot 10 1`; do
  ~/bin/horus-key FILENAME -b $depth >> $logfile
done

grep 'q/s' $logfile > $datafile

gnuplot $gpfile

