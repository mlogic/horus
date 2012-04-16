#!/bin/sh

lasvegas2=128.114.52.64
lasvegas3=128.114.52.65
lasvegas4=128.114.52.66

client=$lasvegas4

touch /tmp/FILENAME
~/bin/horus-file /tmp/FILENAME master-key "yasu in lasvegas"
~/bin/horus-file /tmp/FILENAME kht-block-sizes 4G 1G 256M 64M 16M 4M 1M 256K 64K 16K 4K
~/bin/horus-file /tmp/FILENAME client clear
~/bin/horus-file /tmp/FILENAME client add $client 0 4G

~/bin/horus-file /tmp/FILENAME show

