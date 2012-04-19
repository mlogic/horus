#!/bin/sh

testname=kht-branch-depth-a2

bindir=/home/yasu/bin
srcdir=/home/yasu/horus
workdir=/tmp/horus

lasvegas2=128.114.52.64
lasvegas3=128.114.52.65
lasvegas4=128.114.52.66
lasvegas5=128.114.52.88

server1=$lasvegas2
server2=$lasvegas3
client=$lasvegas4
server3=$lasvegas5

eval_command="$bindir/kds_client -o 0 -l 4G -e -v -b -w -t 1 -n 64 -a 2"

$eval_command -s $server1 $workdir/FILEB2D2  |& tee    ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D3  |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D4  |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D5  |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D6  |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D7  |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D8  |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D9  |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D10 |& tee -a ${testname}-branch2.log
$eval_command -s $server1 $workdir/FILEB2D11 |& tee -a ${testname}-branch2.log
awk -f $srcdir/etc/kht-branch-depth.awk ${testname}-branch2.log > ${testname}-branch2.data


$eval_command -s $server1 $workdir/FILEB4D2  |& tee    ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D3  |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D4  |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D5  |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D6  |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D7  |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D8  |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D9  |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D10 |& tee -a ${testname}-branch4.log
$eval_command -s $server1 $workdir/FILEB4D11 |& tee -a ${testname}-branch4.log
awk -f $srcdir/etc/kht-branch-depth.awk ${testname}-branch4.log > ${testname}-branch4.data


$eval_command -s $server1 $workdir/FILEB6D2  |& tee    ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D3  |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D4  |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D5  |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D6  |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D7  |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D8  |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D9  |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D10 |& tee -a ${testname}-branch6.log
$eval_command -s $server1 $workdir/FILEB6D11 |& tee -a ${testname}-branch6.log
awk -f $srcdir/etc/kht-branch-depth.awk ${testname}-branch6.log > ${testname}-branch6.data


$eval_command -s $server1 $workdir/FILEB8D2  |& tee    ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D3  |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D4  |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D5  |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D6  |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D7  |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D8  |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D9  |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D10 |& tee -a ${testname}-branch8.log
$eval_command -s $server1 $workdir/FILEB8D11 |& tee -a ${testname}-branch8.log
awk -f $srcdir/etc/kht-branch-depth.awk ${testname}-branch8.log > ${testname}-branch8.data



