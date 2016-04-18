#!/usr/bin/env python
#
# Generate a barchart by using Matplotlib
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

import sys
import numpy as np
import matplotlib
matplotlib.use('PDF')
import matplotlib.pyplot as plt
import csv

if len(sys.argv) < 8:
    print '''Usage: barchart.py pdffile title x_label y_label num_of_dataset [datasets...]
For each dataset: csvfile x_column_name y_column_name color legend
All datasets are expected to have same number of rows'''
    sys.exit(2)

pdffile_name = sys.argv[1]
title = sys.argv[2]
x_label = sys.argv[3]
y_label = sys.argv[4]
num_of_dataset = int(sys.argv[5])

# create figure and subplot
width = 0.3       # the width of the bars
fig = plt.figure()
ax = fig.add_subplot(111)
    # add labels
ax.set_ylabel(y_label)
ax.set_xlabel(x_label)
ax.set_title(title)
def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x()+rect.get_width()/2., 1.05*height, '%d'%int(height),
                ha='center', va='bottom')

# Sequence ID of the dataset we are going to process
dataset_id = 0 
argv_index = 6
legend_rect = []
legend_text = []
while argv_index + 3 < len(sys.argv):
    csvfile_name = sys.argv[argv_index]
    argv_index += 1
    x_column_name = sys.argv[argv_index]
    argv_index += 1
    y_column_name = sys.argv[argv_index]
    argv_index += 1
    barcolor = sys.argv[argv_index]
    argv_index += 1
    legend = sys.argv[argv_index]
    argv_index += 1

    csvfile = open(csvfile_name, 'rb')
    reader = csv.DictReader(csvfile)

    x_column = []
    y_column = []
    for data in reader:
        x_column.append(data[x_column_name])
        y_column.append(int(data[y_column_name]))
    N = len(x_column)
    ind = np.arange(N)  # the x locations for the groups
    ax.set_xticks(ind+width * (num_of_dataset/2.0))
    ax.set_xticklabels(x_column)


    # For error bar, add yerr=data_std
    rects = ax.bar(ind + width * dataset_id, y_column, width, color=barcolor)
    legend_rect.append(rects[0])
    legend_text.append(legend)

    autolabel(rects)
    dataset_id += 1

ax.legend(legend_rect, legend_text, loc=5)
plt.savefig(pdffile_name, format='pdf')
# plt.show()
