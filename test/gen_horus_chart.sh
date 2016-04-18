#!/bin/bash
# Generate benchmark charts
#
# Developers:
#  Yan Li <yanli@ascar.io>
#
# Copyright (c) 2012, The Regents of the University of California
# All rights reserved.
# Created by the Storage Systems Research Center, http://www.ssrc.ucsc.edu
# Department of Computer Science
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Storage Systems Research Center, the
#       University of California, nor the names of its contributors
#       may be used to endorse or promote products derived from this
#       software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.

set -e -u

# Use Enthought Python if possible
if which epython >/dev/null; then
    PYTHON=epython
else
    PYTHON=python
fi

# # Figure: number of hashes needed for a 2GB file with branching factor = 2
# ONE_MB_CSV_FILE=1mb_key_calc_count.csv
# # Divide the file according to block sizes
# ./key_count.py > $ONE_MB_CSV_FILE
# head -1 $ONE_MB_CSV_FILE > 1mb_key_calc_count_bs_4kb.csv
# grep ",4096," $ONE_MB_CSV_FILE >>1mb_key_calc_count_bs_4kb.csv
# head -1 $ONE_MB_CSV_FILE > 1mb_key_calc_count_bs_128.csv
# grep ",128," $ONE_MB_CSV_FILE >>1mb_key_calc_count_bs_128.csv
# head -1 $ONE_MB_CSV_FILE > 1mb_key_calc_count_bs_512.csv
# grep ",512," $ONE_MB_CSV_FILE >>1mb_key_calc_count_bs_512.csv

# $PYTHON ./barchart.py hash_count_of_various_depth_with_branching_factor_2.pdf "branching factor = 2" \
#     "KHT Depth" "Hashes needed" 3 \
#     1mb_key_calc_count_bs_4kb.csv kht_depth hash_count "r" "bs=4096" \
#     1mb_key_calc_count_bs_512.csv kht_depth hash_count "g" "bs=512" \
#     1mb_key_calc_count_bs_128.csv kht_depth hash_count "b" "bs=128"

# Figure: benchmark key utils, run for 20 times
# BENCHMARK_KEY_UTILS_OUTPUT_FILE=benchmark_key_utils.csv
# rm -f $BENCHMARK_KEY_UTILS_OUTPUT_FILE
# for ((i=0;i<20;i++)); do
#     ./benchmark_key_utils >>$BENCHMARK_KEY_UTILS_OUTPUT_FILE
# done

# Figure: benchmark read/write 2GB file
BENCHMARK_READ_WRITE_OUTPUT_FILE=benchmark_read_write.csv
rm -f $BENCHMARK_READ_WRITE_OUTPUT_FILE
# warm up
./benchmark_read_write
./benchmark_read_write
for ((i=0;i<20;i++)); do
    ./benchmark_read_write >>$BENCHMARK_READ_WRITE_OUTPUT_FILE
done
