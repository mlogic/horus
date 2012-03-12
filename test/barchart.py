#!/usr/bin/env python

# Generate a barchart by using Matplotlib
#
# Developers:
#  Yan Li <yanli@ucsc.edu>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as 
# published by the Free Software Foundation.

import sys
import numpy as np
import matplotlib
matplotlib.use('PDF')
import matplotlib.pyplot as plt
import csv

if len(sys.argv) < 8:
    print '''Usage: barchart.py csvfile pdffile x_column_name data_column_name title x_label y_label'''
    sys.exit(2)

csvfile_name = sys.argv[1]
pdffile_name = sys.argv[2]
x_column_name = sys.argv[3]
data_column_name = sys.argv[4]
title = sys.argv[5]
x_label = sys.argv[6]
y_label = sys.argv[7]

csvfile = open(sys.argv[1], "rb")
reader = csv.DictReader(csvfile)

x_column = []
data_column = []
for data in reader:
    x_column.append(data[x_column_name])
    data_column.append(int(data[data_column_name]))
N = len(x_column)

ind = np.arange(N)  # the x locations for the groups
width = 0.35       # the width of the bars

fig = plt.figure()
ax = fig.add_subplot(111)
# For error bar, add yerr=data_std
rects1 = ax.bar(ind, data_column, width, color='gray')

# add some labels
ax.set_ylabel(y_label)
ax.set_xlabel(x_label)
ax.set_title(title)
ax.set_xticks(ind+width/2)
ax.set_xticklabels(x_column)

def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x()+rect.get_width()/2., 1.05*height, '%d'%int(height),
                ha='center', va='bottom')

autolabel(rects1)

plt.savefig(pdffile_name, format='pdf')
# plt.show()
