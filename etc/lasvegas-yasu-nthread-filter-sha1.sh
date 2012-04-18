#!/bin/sh

testname=lasvegas-yasu-nthread-filter-sha1

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

touch /tmp/FALSEONSERVER
$bindir/horus-file /tmp/FALSEONSERVER master-key "yasu in lasvegas"
$bindir/horus-file /tmp/FALSEONSERVER kht-block-sizes 4G 1G 256M 64M 16M 4M 1M 256K 64K 16K 4K
$bindir/horus-file /tmp/FALSEONSERVER client clear
$bindir/horus-file /tmp/FALSEONSERVER client add $client 0 4G

$bindir/horus-file /tmp/FALSEONSERVER show

sleep 2;

eval_command="$bindir/kds_client /tmp/FILENAME -x 1 -y 0 -e -v -b -w -t 1"
eval_command2="$bindir/kds_client /tmp/FALSEONSERVER -x 1 -y 0 -e -v -b -w -t 1"

$eval_command -n 16 -s $server1 |& tee ${testname}-server1.log
$eval_command -n 32 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -n 48 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -n 64 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -n 80 -s $server1 |& tee -a ${testname}-server1.log
$eval_command -n 96 -s $server1 |& tee -a ${testname}-server1.log
awk -f $srcdir/etc/${testname}.awk ${testname}-server1.log > ${testname}-server1.data

$eval_command2 -n 16 -s $server1 |& tee ${testname}-server1-false.log
$eval_command2 -n 32 -s $server1 |& tee -a ${testname}-server1-false.log
$eval_command2 -n 48 -s $server1 |& tee -a ${testname}-server1-false.log
$eval_command2 -n 64 -s $server1 |& tee -a ${testname}-server1-false.log
$eval_command2 -n 80 -s $server1 |& tee -a ${testname}-server1-false.log
$eval_command2 -n 96 -s $server1 |& tee -a ${testname}-server1-false.log
awk -f $srcdir/etc/${testname}.awk ${testname}-server1-false.log > ${testname}-server1-false.data

$eval_command -n 16 -s $server1 -s $server2 |& tee ${testname}-server2.log
$eval_command -n 32 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -n 48 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -n 64 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -n 80 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
$eval_command -n 96 -s $server1 -s $server2 |& tee -a ${testname}-server2.log
awk -f $srcdir/etc/${testname}.awk ${testname}-server2.log > ${testname}-server2.data

$eval_command -n 16 -s $server1 -s $server2 -s $server3 |& tee ${testname}-server3.log
$eval_command -n 32 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -n 48 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -n 64 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -n 80 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
$eval_command -n 96 -s $server1 -s $server2 -s $server3 |& tee -a ${testname}-server3.log
awk -f $srcdir/etc/${testname}.awk ${testname}-server3.log > ${testname}-server3.data



