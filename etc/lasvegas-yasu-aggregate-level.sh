#!/bin/sh

testname=lasvegas-yasu-aggregate-level

bindir=/home/yasu/bin
srcdir=/home/yasu/horus

lasvegas2=128.114.52.64
lasvegas3=128.114.52.65
lasvegas4=128.114.52.66
lasvegas5=128.114.52.88

server1=$lasvegas2
server2=$lasvegas3
client=$lasvegas4
server3=$lasvegas5

eval_command="$bindir/kds_client /tmp/FILENAME -x 0 -y 0 -e -v -b -w -t 1 -n 64"

$eval_command -a 10 -s $server1 |& tee ${testname}-server1.log
$eval_command -a 9 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 8 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 7 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 6 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 5 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 4 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 3 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 2 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 1 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -a 0 -s $server1 |& tee -a ${testname}-server1.log
awk -f $srcdir/etc/${testname}.awk ${testname}-server1.log > ${testname}-server1.data

$eval_command -a 10 -s $server1 -s $server2 |& tee ${testname}-server2.log
$eval_command -a 9 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 8 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 7 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 6 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 5 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 4 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 3 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 2 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 1 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -a 0 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
awk -f $srcdir/etc/${testname}.awk ${testname}-server2.log > ${testname}-server2.data

$eval_command -a 10 -s $server1 -s $server2 -s $server3 |& tee ${testname}-server3.log
$eval_command -a 9 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 8 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 7 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 6 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 5 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 4 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 3 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 2 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 1 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -a 0 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
awk -f $srcdir/etc/${testname}.awk ${testname}-server3.log > ${testname}-server3.data


