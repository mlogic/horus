#!/usr/bin/env python

# Calculate number of hashes needed for a KHT setting
#
# Developers:
#  Yan Li <yanli@ucsc.edu>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as 
# published by the Free Software Foundation.

#kht_depth = input('Input KHT dpeth: ')
#branching_factor = input('Input branching factor: ');
#file_size = input('Input file size: ')
#block_size = input('Block size: ')

gb = 1024 * 1024 * 1024
kb = 1024
debug = 1

def get_hash_count(kht_depth, branching_factor, file_size, block_size):
    if debug == 1:
        print '%s,%s,%s,%s,' % \
            (kht_depth, branching_factor, file_size, block_size),
    nblock = file_size / block_size
    hash_count = 0
    for i in range(0, kht_depth):
        hash_count += nblock
        # get the ceiling of nblock/branching_factor so we won't miss the
        # last non-full tree
        if nblock % branching_factor != 0:
            nblock = nblock / branching_factor + 1
        else:
            nblock = nblock / branching_factor

    if debug == 1:
        print hash_count
    return hash_count

# Figure: number of hashes needed for a 2GB file with branching factor = 2
print 'kht_depth,branching_factor,file_size,block_size,hash_count'
get_hash_count(2, 2, 2*gb, 4*kb)
get_hash_count(4, 2, 2*gb, 4*kb)
get_hash_count(6, 2, 2*gb, 4*kb)
get_hash_count(8, 2, 2*gb, 4*kb)
get_hash_count(10, 2, 2*gb, 4*kb)
