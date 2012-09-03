#!/usr/bin/env python

import os
import sys
import logging
import re
import csv

import matplotlib.pyplot as plt
import numpy

from picoquant import histogram, modes

def mean(L):
    return(sum(L)/len(L))

def g3_from_stream(stream):
    for line in stream:
        yield(histogram.HistogramBin(mode=modes.T2, order=3,
                                     string=line))

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        with open(filename) as stream:
            g3_bins = list(g3_from_stream(stream))

        time_axis = list(
            sorted(
                set(
                    map(lambda x: x.time_bins()[0][0].bounds,
                        filter(lambda y: y.correlation() == (0,1,2),
                               g3_bins)))))

        print(time_axis)
        
        g3 = list()
        for t in reversed(time_axis[1:-1]):
            g3.append(
                list(map(lambda x: x.counts,
                         list(
                             filter(lambda y: y.time_bins()[0][0].bounds == t,
                                    g3_bins))[1:-1])))
                    
        g3 = numpy.array(g3)
        print(g3)

        time_centers = list(map(lambda x: mean(x), time_axis))
        print(time_centers)
        plt.clf()
        plt.imshow(g3,
                   extent=(time_centers[1], time_centers[-1],
                           time_centers[1], time_centers[-1]),
                   interpolation="nearest")
        plt.xlabel(r"$\Delta p_{1}$")
        plt.ylabel(r"$\Delta p_{2}$")
        plt.title(r"$g^{(3)}(\Delta p_{1}, \Delta p_{2})$")
        plt.colorbar()
#        plt.savefig("g3_4x.png")
        plt.show()
