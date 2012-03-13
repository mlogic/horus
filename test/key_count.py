#!/usr/bin/env python

# Calculate number of hashes needed for a KHT setting
#
# Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Developers:
#  Yan Li <yanli@ucsc.edu>


#kht_depth = input('Input KHT dpeth: ')
#branching_factor = input('Input branching factor: ');
#file_size = input('Input file size: ')
#block_size = input('Block size: ')

kb = 1024
mb = 1024 * kb
gb = 1024 * mb
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

# Figure: number of hashes needed for a 1MB block with branching factor = 2
print 'kht_depth,branching_factor,file_size,block_size,hash_count'
get_hash_count(2, 2, mb, 4*kb)
get_hash_count(4, 2, mb, 4*kb)
get_hash_count(6, 2, mb, 4*kb)
get_hash_count(8, 2, mb, 4*kb)
get_hash_count(10, 2, mb, 4*kb)
get_hash_count(2, 2, mb, 128)
get_hash_count(4, 2, mb, 128)
get_hash_count(6, 2, mb, 128)
get_hash_count(8, 2, mb, 128)
get_hash_count(10, 2, mb, 128)
get_hash_count(2, 2, mb, 512)
get_hash_count(4, 2, mb, 512)
get_hash_count(6, 2, mb, 512)
get_hash_count(8, 2, mb, 512)
get_hash_count(10, 2, mb, 512)
