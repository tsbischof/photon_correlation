#!/usr/bin/env python

import os
import csv
import sys
import subprocess
import matplotlib.pyplot as plt

import process_picoquant

def get_curves(data):
    curves = dict()
    for channel_0, channel_1, bin_edge, counts in csv.reader(data):
        key = "{0}_{1}".format(channel_0, channel_1)
        if not key in curves.keys():
            curves[key] = list()

        curves[key].append((float(bin_edge), int(counts)))

    return(curves)

def plot_curves(curves):
    for key, curve in sorted(curves.items()):
        try:           
            print(key)
            channel_0, channel_1 = key.split("_")
            times = map(lambda x: x[0], curve)
            counts = map(lambda x: x[1], curve)
            name = r"$g_{%s%s}$($\tau$)" % (channel_0, channel_1)

            plt.clf()
            plt.semilogy(times, counts)
            plt.xlim((min(times), max(times)))
            plt.xlabel("Time/ns")
            plt.ylabel("Counts")
            plt.title(name)
            plt.show()
        except ValueError:
            pass


def plot_histograms(data):
    curves = get_curves(data)
    plot_curves(curves)

if __name__ == "__main__":
#    with open("../src/blargh") as data:
#        plot_histograms(data)
    plot_histograms(sys.stdin)
