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
        name = names[key]
        wavelengths = list(map(lambda x: x[0], curve))
        counts = list(map(lambda x: x[1], curve))

        plt.clf()
        plt.semilogy(wavelengths, counts, label=name)
        plt.xlabel("Time/nm")
        plt.ylabel("Counts")
        plt.title(name)
        plt.show()    

def plot_histograms(filename, names=None):
    data = subprocess.Popen([picoquant, "--file-in", filename],
                            stdout=subprocess.PIPE)
    curves = get_curves(data.stdout)
    if names == None:
        names = dict()
        for key in curves.keys():
            names[key] = key
            
    plot_curves(curves, names)

if __name__ == "__main__":
    filenames = ["../sample_data/Coum39_LDH400.thd"]

    for filename in filenames:
        plot_histograms(filename)
