#!/bin/sh

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

mkdir $workdir

touch $workdir/FILEB2D2
touch $workdir/FILEB2D3
touch $workdir/FILEB2D4
touch $workdir/FILEB2D5
touch $workdir/FILEB2D6
touch $workdir/FILEB2D7
touch $workdir/FILEB2D8
touch $workdir/FILEB2D9
touch $workdir/FILEB2D10
touch $workdir/FILEB2D11

$bindir/horus-file $workdir/FILEB2D11 kht-block-sizes 4M 2M 1M 512K 256K 128K 64K 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D10 kht-block-sizes 2M 1M 512K 256K 128K 64K 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D9 kht-block-sizes 1M 512K 256K 128K 64K 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D8 kht-block-sizes 512K 256K 128K 64K 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D7 kht-block-sizes 256K 128K 64K 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D6 kht-block-sizes 128K 64K 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D5 kht-block-sizes 64K 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D4 kht-block-sizes 32K 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D3 kht-block-sizes 16K 8K 4K
$bindir/horus-file $workdir/FILEB2D2 kht-block-sizes 8K 4K

touch $workdir/FILEB4D2
touch $workdir/FILEB4D3
touch $workdir/FILEB4D4
touch $workdir/FILEB4D5
touch $workdir/FILEB4D6
touch $workdir/FILEB4D7
touch $workdir/FILEB4D8
touch $workdir/FILEB4D9
touch $workdir/FILEB4D10
touch $workdir/FILEB4D11


$bindir/horus-file $workdir/FILEB4D11 kht-block-sizes 4G 1G 256M 64M 16M 4M 1M 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D10 kht-block-sizes 1G 256M 64M 16M 4M 1M 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D9 kht-block-sizes 256M 64M 16M 4M 1M 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D8 kht-block-sizes 64M 16M 4M 1M 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D7 kht-block-sizes 16M 4M 1M 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D6 kht-block-sizes 4M 1M 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D5 kht-block-sizes 1M 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D4 kht-block-sizes 256K 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D3 kht-block-sizes 64K 16K 4K
$bindir/horus-file $workdir/FILEB4D2 kht-block-sizes 16K 4K

touch $workdir/FILEB6D2
touch $workdir/FILEB6D3
touch $workdir/FILEB6D4
touch $workdir/FILEB6D5
touch $workdir/FILEB6D6
touch $workdir/FILEB6D7
touch $workdir/FILEB6D8
touch $workdir/FILEB6D9
touch $workdir/FILEB6D10
touch $workdir/FILEB6D11

$bindir/horus-file $workdir/FILEB6D11 kht-block-sizes 236196M 39366M 6561M 1119744K 186624K 31104K 5184K 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D10 kht-block-sizes 39366M 6561M 1119744K 186624K 31104K 5184K 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D9 kht-block-sizes 6561M 1119744K 186624K 31104K 5184K 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D8 kht-block-sizes 1119744K 186624K 31104K 5184K 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D7 kht-block-sizes 186624K 31104K 5184K 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D6 kht-block-sizes 31104K 5184K 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D5 kht-block-sizes 5184K 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D4 kht-block-sizes 864K 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D3 kht-block-sizes 144K 24K 4K
$bindir/horus-file $workdir/FILEB6D2 kht-block-sizes 24K 4K

touch $workdir/FILEB8D2
touch $workdir/FILEB8D3
touch $workdir/FILEB8D4
touch $workdir/FILEB8D5
touch $workdir/FILEB8D6
touch $workdir/FILEB8D7
touch $workdir/FILEB8D8
touch $workdir/FILEB8D9
touch $workdir/FILEB8D10
touch $workdir/FILEB8D11

$bindir/horus-file $workdir/FILEB8D11 kht-block-sizes 4096G 512G 64G 8G 1G 128M 16M 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D10 kht-block-sizes 512G 64G 8G 1G 128M 16M 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D9 kht-block-sizes 64G 8G 1G 128M 16M 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D8 kht-block-sizes 8G 1G 128M 16M 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D7 kht-block-sizes 1G 128M 16M 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D6 kht-block-sizes 128M 16M 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D5 kht-block-sizes 16M 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D4 kht-block-sizes 2M 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D3 kht-block-sizes 256K 32K 4K
$bindir/horus-file $workdir/FILEB8D2 kht-block-sizes 32K 4K


for i in `ls $workdir`; do
  $bindir/horus-file $workdir/$i master-key "yasu in lasvegas"
  $bindir/horus-file $workdir/$i client clear
  $bindir/horus-file $workdir/$i client add $client 0 4096G
done



