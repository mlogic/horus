#!/bin/sh

target=/scratch/horus/horus-data-file

bindir=~yasu/bin
horus_file=$bindir/horus-file

touch $target
$horus_file $target master-key "horus master key"
$horus_file $target kht-block-sizes 4096G 512G 64G 8G 1G 128M 16M 2M 256K 32K 4K

$horus_file $target client clear
$horus_file $target client add 128.114.52.63 0 4096G
$horus_file $target client add 128.114.52.64 0 4096G
$horus_file $target client add 128.114.52.65 0 4096G
$horus_file $target client add 128.114.52.66 0 4096G
$horus_file $target client add 128.114.52.88 0 4096G
$horus_file $target client add 128.114.52.89 0 4096G
$horus_file $target client add 128.114.52.90 0 4096G
$horus_file $target client add 128.114.52.91 0 4096G
$horus_file $target client add 128.114.52.92 0 4096G
$horus_file $target client add 128.114.52.93 0 4096G
$horus_file $target client add 128.114.52.94 0 4096G
$horus_file $target client add 128.114.52.143 0 4096G
$horus_file $target client add 128.114.52.144 0 4096G
$horus_file $target client add 128.114.52.145 0 4096G
$horus_file $target client add 128.114.52.146 0 4096G
$horus_file $target client add 128.114.52.147 0 4096G
$horus_file $target client add 128.114.52.148 0 4096G
$horus_file $target client add 128.114.52.149 0 4096G
$horus_file $target client add 128.114.52.150 0 4096G
$horus_file $target client add 128.114.52.151 0 4096G





