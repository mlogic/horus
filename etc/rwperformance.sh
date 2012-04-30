#!/bin/sh

testname=rwperformance

bindir=~/bin
srcdir=~/horus
workdir=/tmp/horus

. $srcdir/etc/server-conf.sh

touch $workdir/FILEB8D11
$bindir/horus-file $workdir/FILEB8D11 kht-block-sizes 4096G 512G 64G 8G 1G 128M 16M 2M 256K 32K 4K
# dd if=/dev/zero of=/$workdir/FILEB8D11 bs=4096 count=1048576


cmd="$bindir/horus-read-write -s $server1 -b -r"

$cmd $workdir/FILEB8D11 -l  512M 2>&1 | tee    ${testname}-read.log
$cmd $workdir/FILEB8D11 -l 1024M 2>&1 | tee -a ${testname}-read.log
$cmd $workdir/FILEB8D11 -l 1536M 2>&1 | tee -a ${testname}-read.log
$cmd $workdir/FILEB8D11 -l 2048M 2>&1 | tee -a ${testname}-read.log
$cmd $workdir/FILEB8D11 -l 2560M 2>&1 | tee -a ${testname}-read.log
$cmd $workdir/FILEB8D11 -l 3072M 2>&1 | tee -a ${testname}-read.log
$cmd $workdir/FILEB8D11 -l 3584M 2>&1 | tee -a ${testname}-read.log
$cmd $workdir/FILEB8D11 -l 4096M 2>&1 | tee -a ${testname}-read.log
awk -f $srcdir/etc/${testname}.awk ${testname}-read.log > ${testname}-read.data

$cmd $workdir/FILEB8D11 -l  512M -g 2>&1 | tee    ${testname}-eread.log
$cmd $workdir/FILEB8D11 -l 1024M -g 2>&1 | tee -a ${testname}-eread.log
$cmd $workdir/FILEB8D11 -l 1536M -g 2>&1 | tee -a ${testname}-eread.log
$cmd $workdir/FILEB8D11 -l 2048M -g 2>&1 | tee -a ${testname}-eread.log
$cmd $workdir/FILEB8D11 -l 2560M -g 2>&1 | tee -a ${testname}-eread.log
$cmd $workdir/FILEB8D11 -l 3072M -g 2>&1 | tee -a ${testname}-eread.log
$cmd $workdir/FILEB8D11 -l 3584M -g 2>&1 | tee -a ${testname}-eread.log
$cmd $workdir/FILEB8D11 -l 4096M -g 2>&1 | tee -a ${testname}-eread.log
awk -f $srcdir/etc/${testname}.awk ${testname}-eread.log > ${testname}-eread.data

$cmd $workdir/FILEB8D11 -l  512M -g -u 2>&1 | tee    ${testname}-euread.log
$cmd $workdir/FILEB8D11 -l 1024M -g -u 2>&1 | tee -a ${testname}-euread.log
$cmd $workdir/FILEB8D11 -l 1536M -g -u 2>&1 | tee -a ${testname}-euread.log
$cmd $workdir/FILEB8D11 -l 2048M -g -u 2>&1 | tee -a ${testname}-euread.log
$cmd $workdir/FILEB8D11 -l 2560M -g -u 2>&1 | tee -a ${testname}-euread.log
$cmd $workdir/FILEB8D11 -l 3072M -g -u 2>&1 | tee -a ${testname}-euread.log
$cmd $workdir/FILEB8D11 -l 3584M -g -u 2>&1 | tee -a ${testname}-euread.log
$cmd $workdir/FILEB8D11 -l 4096M -g -u 2>&1 | tee -a ${testname}-euread.log
awk -f $srcdir/etc/${testname}.awk ${testname}-euread.log > ${testname}-euread.data

cmd="$bindir/horus-read-write -s $server1 -b -r -a 3"
$cmd $workdir/FILEB8D11 -l  512M -g -u 2>&1 | tee    ${testname}-euread-a3.log
$cmd $workdir/FILEB8D11 -l 1024M -g -u 2>&1 | tee -a ${testname}-euread-a3.log
$cmd $workdir/FILEB8D11 -l 1536M -g -u 2>&1 | tee -a ${testname}-euread-a3.log
$cmd $workdir/FILEB8D11 -l 2048M -g -u 2>&1 | tee -a ${testname}-euread-a3.log
$cmd $workdir/FILEB8D11 -l 2560M -g -u 2>&1 | tee -a ${testname}-euread-a3.log
$cmd $workdir/FILEB8D11 -l 3072M -g -u 2>&1 | tee -a ${testname}-euread-a3.log
$cmd $workdir/FILEB8D11 -l 3584M -g -u 2>&1 | tee -a ${testname}-euread-a3.log
$cmd $workdir/FILEB8D11 -l 4096M -g -u 2>&1 | tee -a ${testname}-euread-a3.log
awk -f $srcdir/etc/${testname}.awk ${testname}-euread-a3.log > ${testname}-euread-a3.data


cmd="$bindir/horus-read-write -s $server1 -b -w"

$cmd $workdir/FILEB8D11 -l  512M 2>&1 | tee    ${testname}-write.log
$cmd $workdir/FILEB8D11 -l 1024M 2>&1 | tee -a ${testname}-write.log
$cmd $workdir/FILEB8D11 -l 1536M 2>&1 | tee -a ${testname}-write.log
$cmd $workdir/FILEB8D11 -l 2048M 2>&1 | tee -a ${testname}-write.log
$cmd $workdir/FILEB8D11 -l 2560M 2>&1 | tee -a ${testname}-write.log
$cmd $workdir/FILEB8D11 -l 3072M 2>&1 | tee -a ${testname}-write.log
$cmd $workdir/FILEB8D11 -l 3584M 2>&1 | tee -a ${testname}-write.log
$cmd $workdir/FILEB8D11 -l 4096M 2>&1 | tee -a ${testname}-write.log
awk -f $srcdir/etc/${testname}.awk ${testname}-write.log > ${testname}-write.data

$cmd $workdir/FILEB8D11 -l  512M -e 2>&1 | tee    ${testname}-ewrite.log
$cmd $workdir/FILEB8D11 -l 1024M -e 2>&1 | tee -a ${testname}-ewrite.log
$cmd $workdir/FILEB8D11 -l 1536M -e 2>&1 | tee -a ${testname}-ewrite.log
$cmd $workdir/FILEB8D11 -l 2048M -e 2>&1 | tee -a ${testname}-ewrite.log
$cmd $workdir/FILEB8D11 -l 2560M -e 2>&1 | tee -a ${testname}-ewrite.log
$cmd $workdir/FILEB8D11 -l 3072M -e 2>&1 | tee -a ${testname}-ewrite.log
$cmd $workdir/FILEB8D11 -l 3584M -e 2>&1 | tee -a ${testname}-ewrite.log
$cmd $workdir/FILEB8D11 -l 4096M -e 2>&1 | tee -a ${testname}-ewrite.log
awk -f $srcdir/etc/${testname}.awk ${testname}-ewrite.log > ${testname}-ewrite.data

$cmd $workdir/FILEB8D11 -l  512M -e -u 2>&1 | tee    ${testname}-euwrite.log
$cmd $workdir/FILEB8D11 -l 1024M -e -u 2>&1 | tee -a ${testname}-euwrite.log
$cmd $workdir/FILEB8D11 -l 1536M -e -u 2>&1 | tee -a ${testname}-euwrite.log
$cmd $workdir/FILEB8D11 -l 2048M -e -u 2>&1 | tee -a ${testname}-euwrite.log
$cmd $workdir/FILEB8D11 -l 2560M -e -u 2>&1 | tee -a ${testname}-euwrite.log
$cmd $workdir/FILEB8D11 -l 3072M -e -u 2>&1 | tee -a ${testname}-euwrite.log
$cmd $workdir/FILEB8D11 -l 3584M -e -u 2>&1 | tee -a ${testname}-euwrite.log
$cmd $workdir/FILEB8D11 -l 4096M -e -u 2>&1 | tee -a ${testname}-euwrite.log
awk -f $srcdir/etc/${testname}.awk ${testname}-euwrite.log > ${testname}-euwrite.data

cmd="$bindir/horus-read-write -s $server1 -b -w -a 3"
$cmd $workdir/FILEB8D11 -l  512M -e -u 2>&1 | tee    ${testname}-euwrite-a3.log
$cmd $workdir/FILEB8D11 -l 1024M -e -u 2>&1 | tee -a ${testname}-euwrite-a3.log
$cmd $workdir/FILEB8D11 -l 1536M -e -u 2>&1 | tee -a ${testname}-euwrite-a3.log
$cmd $workdir/FILEB8D11 -l 2048M -e -u 2>&1 | tee -a ${testname}-euwrite-a3.log
$cmd $workdir/FILEB8D11 -l 2560M -e -u 2>&1 | tee -a ${testname}-euwrite-a3.log
$cmd $workdir/FILEB8D11 -l 3072M -e -u 2>&1 | tee -a ${testname}-euwrite-a3.log
$cmd $workdir/FILEB8D11 -l 3584M -e -u 2>&1 | tee -a ${testname}-euwrite-a3.log
$cmd $workdir/FILEB8D11 -l 4096M -e -u 2>&1 | tee -a ${testname}-euwrite-a3.log
awk -f $srcdir/etc/${testname}.awk ${testame}-euwrite-a3.log > ${testname}-euwrite-a3.data



