#!/usr/bin/env python

import subprocess
import csv
import sys

import matplotlib.pyplot as plt

picoquant = "/home/tsbischof/src/picoquant/src/picoquant"

def get_curves(data):
    curves = dict()
    for curve, time, counts in csv.reader(data):
        if curve not in curves.keys():
            curves[curve] = list()

        curves[curve].append([float(time), int(counts)])
    return(curves)

def plot_curves(curves, names):
    for key, curve in sorted(curves.items()):
        try:
            name = names[key]
            times = list(map(lambda x: x[0], curve))
            counts = list(map(lambda x: x[1], curve))

            plt.clf()
            plt.semilogy(times, counts, label=name)
            plt.xlabel("Time/ns")
            plt.ylabel("Counts")
            plt.title(name)
            plt.show()
        except:
            pass

def plot_histograms(data, names=None):
    curves = get_curves(data)
    if names == None:
        names = dict()
        for key in curves.keys():
            names[key] = key
            
    plot_curves(curves, names)

if __name__ == "__main__":
    plot_histograms(sys.stdin)
