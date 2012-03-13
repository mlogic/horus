#!/bin/bash
set -e -u

# Yan Li's Enthought Python
if which epython >/dev/null; then
    PYTHON=epython
else
    PYTHON=python
fi

# Figure: number of hashes needed for a 2GB file with branching factor = 2
ONE_MB_CSV_FILE=1mb_key_calc_count.csv
# Divide the file according to block sizes
./key_count.py > $ONE_MB_CSV_FILE
head -1 $ONE_MB_CSV_FILE > 1mb_key_calc_count_bs_4kb.csv
grep ",4096," $ONE_MB_CSV_FILE >>1mb_key_calc_count_bs_4kb.csv
head -1 $ONE_MB_CSV_FILE > 1mb_key_calc_count_bs_128.csv
grep ",128," $ONE_MB_CSV_FILE >>1mb_key_calc_count_bs_128.csv
head -1 $ONE_MB_CSV_FILE > 1mb_key_calc_count_bs_512.csv
grep ",512," $ONE_MB_CSV_FILE >>1mb_key_calc_count_bs_512.csv

$PYTHON ./barchart.py hash_count_of_various_depth_with_branching_factor_2.pdf "branching factor = 2" \
    "KHT Depth" "Hashes needed" 3 \
    1mb_key_calc_count_bs_4kb.csv kht_depth hash_count "r" "bs=4096" \
    1mb_key_calc_count_bs_512.csv kht_depth hash_count "g" "bs=512" \
    1mb_key_calc_count_bs_128.csv kht_depth hash_count "b" "bs=128"

# Figure: benchmark key utils
#benchmark_key_utils
