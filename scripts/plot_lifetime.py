#!/usr/bin/env python

import csv
import sys

import matplotlib.pyplot as plt

def plot_lifetime(lifetime):
    histograms = set(map(lambda x: x[0], lifetime))
    plt.clf()

    for histogram in sorted(histograms):
        my_lifetime = filter(lambda x: x[0] == histogram, lifetime)
        times = list(map(lambda x: x[1], my_lifetime))
        counts = list(map(lambda x: x[2], my_lifetime))

        plt.semilogy(times, counts)
    plt.show()

def lifetime_from_file(stream):
    for histogram_index, bin_left, bin_right, counts in stream:
        yield((int(histogram_index),
               float(bin_left),
               int(counts)))

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        with open(filename) as stream:
            lifetimes = list(lifetime_from_file(csv.reader(stream)))

            plot_lifetime(lifetimes)

