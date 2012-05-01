#!/bin/sh

testname=rwperformance

bindir=~/bin
srcdir=~/horus
workdir=/tmp/horus
cephdir=/ceph/horus

awkfile=$srcdir/etc/${testname}.awk
testprefix=${testname}-ceph

. $srcdir/etc/server-conf.sh

touch $workdir/FILEB8D11
$bindir/horus-file $workdir/FILEB8D11 kht-block-sizes 4096G 512G 64G 8G 1G 128M 16M 2M 256K 32K 4K

# dd if=/dev/zero of=/$workdir/FILEB8D11 bs=4096 count=1048576

file=$workdir/FILEB8D11

cmd="$bindir/horus-read-write -s $server1 -b -r -i $cephdir/FILEB8D11"

$cmd $file -l  512M 2>&1 | tee    ${testprefix}-read.log
$cmd $file -l 1024M 2>&1 | tee -a ${testprefix}-read.log
$cmd $file -l 1536M 2>&1 | tee -a ${testprefix}-read.log
$cmd $file -l 2048M 2>&1 | tee -a ${testprefix}-read.log
$cmd $file -l 2560M 2>&1 | tee -a ${testprefix}-read.log
$cmd $file -l 3072M 2>&1 | tee -a ${testprefix}-read.log
$cmd $file -l 3584M 2>&1 | tee -a ${testprefix}-read.log
$cmd $file -l 4096M 2>&1 | tee -a ${testprefix}-read.log
awk -f ${awkfile} ${testprefix}-read.log > ${testprefix}-read.data

$cmd $file -l  512M -g 2>&1 | tee    ${testprefix}-eread.log
$cmd $file -l 1024M -g 2>&1 | tee -a ${testprefix}-eread.log
$cmd $file -l 1536M -g 2>&1 | tee -a ${testprefix}-eread.log
$cmd $file -l 2048M -g 2>&1 | tee -a ${testprefix}-eread.log
$cmd $file -l 2560M -g 2>&1 | tee -a ${testprefix}-eread.log
$cmd $file -l 3072M -g 2>&1 | tee -a ${testprefix}-eread.log
$cmd $file -l 3584M -g 2>&1 | tee -a ${testprefix}-eread.log
$cmd $file -l 4096M -g 2>&1 | tee -a ${testprefix}-eread.log
awk -f ${awkfile} ${testprefix}-eread.log > ${testprefix}-eread.data

$cmd $file -l  512M -g -u 2>&1 | tee    ${testprefix}-euread.log
$cmd $file -l 1024M -g -u 2>&1 | tee -a ${testprefix}-euread.log
$cmd $file -l 1536M -g -u 2>&1 | tee -a ${testprefix}-euread.log
$cmd $file -l 2048M -g -u 2>&1 | tee -a ${testprefix}-euread.log
$cmd $file -l 2560M -g -u 2>&1 | tee -a ${testprefix}-euread.log
$cmd $file -l 3072M -g -u 2>&1 | tee -a ${testprefix}-euread.log
$cmd $file -l 3584M -g -u 2>&1 | tee -a ${testprefix}-euread.log
$cmd $file -l 4096M -g -u 2>&1 | tee -a ${testprefix}-euread.log
awk -f ${awkfile} ${testprefix}-euread.log > ${testprefix}-euread.data

$cmd $file -l  512M -g -u -a 3 2>&1 | tee    ${testprefix}-euread-a3.log
$cmd $file -l 1024M -g -u -a 3 2>&1 | tee -a ${testprefix}-euread-a3.log
$cmd $file -l 1536M -g -u -a 3 2>&1 | tee -a ${testprefix}-euread-a3.log
$cmd $file -l 2048M -g -u -a 3 2>&1 | tee -a ${testprefix}-euread-a3.log
$cmd $file -l 2560M -g -u -a 3 2>&1 | tee -a ${testprefix}-euread-a3.log
$cmd $file -l 3072M -g -u -a 3 2>&1 | tee -a ${testprefix}-euread-a3.log
$cmd $file -l 3584M -g -u -a 3 2>&1 | tee -a ${testprefix}-euread-a3.log
$cmd $file -l 4096M -g -u -a 3 2>&1 | tee -a ${testprefix}-euread-a3.log
awk -f ${awkfile} ${testprefix}-euread-a3.log > ${testprefix}-euread-a3.data


cmd="$bindir/horus-read-write -s $server1 -b -w -o $cephdir/FILEB8D11"

$cmd $file -l  512M 2>&1 | tee    ${testprefix}-write.log
$cmd $file -l 1024M 2>&1 | tee -a ${testprefix}-write.log
$cmd $file -l 1536M 2>&1 | tee -a ${testprefix}-write.log
$cmd $file -l 2048M 2>&1 | tee -a ${testprefix}-write.log
$cmd $file -l 2560M 2>&1 | tee -a ${testprefix}-write.log
$cmd $file -l 3072M 2>&1 | tee -a ${testprefix}-write.log
$cmd $file -l 3584M 2>&1 | tee -a ${testprefix}-write.log
$cmd $file -l 4096M 2>&1 | tee -a ${testprefix}-write.log
awk -f ${awkfile} ${testprefix}-write.log > ${testprefix}-write.data

$cmd $file -l  512M -e 2>&1 | tee    ${testprefix}-ewrite.log
$cmd $file -l 1024M -e 2>&1 | tee -a ${testprefix}-ewrite.log
$cmd $file -l 1536M -e 2>&1 | tee -a ${testprefix}-ewrite.log
$cmd $file -l 2048M -e 2>&1 | tee -a ${testprefix}-ewrite.log
$cmd $file -l 2560M -e 2>&1 | tee -a ${testprefix}-ewrite.log
$cmd $file -l 3072M -e 2>&1 | tee -a ${testprefix}-ewrite.log
$cmd $file -l 3584M -e 2>&1 | tee -a ${testprefix}-ewrite.log
$cmd $file -l 4096M -e 2>&1 | tee -a ${testprefix}-ewrite.log
awk -f ${awkfile} ${testprefix}-ewrite.log > ${testprefix}-ewrite.data

$cmd $file -l  512M -e -u 2>&1 | tee    ${testprefix}-euwrite.log
$cmd $file -l 1024M -e -u 2>&1 | tee -a ${testprefix}-euwrite.log
$cmd $file -l 1536M -e -u 2>&1 | tee -a ${testprefix}-euwrite.log
$cmd $file -l 2048M -e -u 2>&1 | tee -a ${testprefix}-euwrite.log
$cmd $file -l 2560M -e -u 2>&1 | tee -a ${testprefix}-euwrite.log
$cmd $file -l 3072M -e -u 2>&1 | tee -a ${testprefix}-euwrite.log
$cmd $file -l 3584M -e -u 2>&1 | tee -a ${testprefix}-euwrite.log
$cmd $file -l 4096M -e -u 2>&1 | tee -a ${testprefix}-euwrite.log
awk -f ${awkfile} ${testprefix}-euwrite.log > ${testprefix}-euwrite.data

$cmd $file -l  512M -e -u -a 3 2>&1 | tee    ${testprefix}-euwrite-a3.log
$cmd $file -l 1024M -e -u -a 3 2>&1 | tee -a ${testprefix}-euwrite-a3.log
$cmd $file -l 1536M -e -u -a 3 2>&1 | tee -a ${testprefix}-euwrite-a3.log
$cmd $file -l 2048M -e -u -a 3 2>&1 | tee -a ${testprefix}-euwrite-a3.log
$cmd $file -l 2560M -e -u -a 3 2>&1 | tee -a ${testprefix}-euwrite-a3.log
$cmd $file -l 3072M -e -u -a 3 2>&1 | tee -a ${testprefix}-euwrite-a3.log
$cmd $file -l 3584M -e -u -a 3 2>&1 | tee -a ${testprefix}-euwrite-a3.log
$cmd $file -l 4096M -e -u -a 3 2>&1 | tee -a ${testprefix}-euwrite-a3.log
awk -f ${awkfile} ${testprefix}-euwrite-a3.log > ${testprefix}-euwrite-a3.data



