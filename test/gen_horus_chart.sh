#!/bin/bash
set -e -u

# Yan Li's Enthought Python
if which epython >/dev/null; then
    PYTHON=epython
else
    PYTHON=python
fi

# Figure: number of hashes needed for a 2GB file with branching factor = 2
./key_count.py > 2gb_key_calc_count.csv 

$PYTHON ./barchart.py 2gb_key_calc_count.csv hash_count_of_various_depth_with_branching_factor_2.pdf kht_depth hash_count "branching factor = 2" "KHT Depth" "Hashes needed"
