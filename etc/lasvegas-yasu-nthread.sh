#!/bin/sh

testname=lasvegas-yasu-nthread

lasvegas2=128.114.52.64
lasvegas3=128.114.52.65
lasvegas4=128.114.52.66

server1=$lasvegas2
server2=$lasvegas3
client=$lasvegas4

~/bin/kds_client /tmp/FILENAME -n 8 -s $server1 -x 1 -y 0 -e -v -b
~/bin/kds_client /tmp/FILENAME -n 16 -s $server1 -x 1 -y 0 -e -v -b
~/bin/kds_client /tmp/FILENAME -n 32 -s $server1 -x 1 -y 0 -e -v -b

~/bin/kds_client /tmp/FILENAME -n 32 -s $server1 -s $server2 -x 1 -y 0 -e -v -b
