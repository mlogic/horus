#!/bin/sh

testname=rwperformance

bindir=/home/yasu/bin
srcdir=/home/yasu/horus
workdir=/tmp/horus

. $srcdir/etc/server-conf.sh

cmd="$bindir/horus-read-write -b -v -a 3"

$cmd -s $server1 $workdir/FILEB8D11 -r -l  512K |& tee    ${testname}-read.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 1024K |& tee -a ${testname}-read.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 1536K |& tee -a ${testname}-read.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 2048K |& tee -a ${testname}-read.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 2560K |& tee -a ${testname}-read.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 3072K |& tee -a ${testname}-read.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 3584K |& tee -a ${testname}-read.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 4096K |& tee -a ${testname}-read.log
awk -f $srcdir/etc/${testname}.awk ${testname}-read.log > ${testname}-read.data


$cmd -s $server1 $workdir/FILEB8D11 -r -l  512K -g |& tee    ${testname}-eread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 1024K -g |& tee -a ${testname}-eread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 1536K -g |& tee -a ${testname}-eread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 2048K -g |& tee -a ${testname}-eread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 2560K -g |& tee -a ${testname}-eread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 3072K -g |& tee -a ${testname}-eread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 3584K -g |& tee -a ${testname}-eread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 4096K -g |& tee -a ${testname}-eread.log
awk -f $srcdir/etc/${testname}.awk ${testname}-eread.log > ${testname}-eread.data

$cmd -s $server1 $workdir/FILEB8D11 -r -l  512K -g -u |& tee    ${testname}-euread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 1024K -g -u |& tee -a ${testname}-euread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 1536K -g -u |& tee -a ${testname}-euread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 2048K -g -u |& tee -a ${testname}-euread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 2560K -g -u |& tee -a ${testname}-euread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 3072K -g -u |& tee -a ${testname}-euread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 3584K -g -u |& tee -a ${testname}-euread.log
$cmd -s $server1 $workdir/FILEB8D11 -r -l 4096K -g -u |& tee -a ${testname}-euread.log
awk -f $srcdir/etc/${testname}.awk ${testname}-euread.log > ${testname}-euread.data


