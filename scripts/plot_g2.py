#!/usr/bin/env python3

import csv
import sys
import math
import argparse

import matplotlib.pyplot as plt

def mean(L):
    return(float(sum(L))/len(L))

def g2_from_stream(stream):
    result = dict()
    for channel_0, channel_1, bin_left, bin_right, counts in csv.reader(stream):
        key = (int(channel_0), int(channel_1))
        if key not in result.keys():
            result[key] = list()

        result[key].append(((float(bin_left), float(bin_right)), float(counts)))

    return(result)

def plot_g2s(g2s, scale="linear"):
    if scale == "linear":
        plotter = plt.plot
    elif scale == "log":
        plotter = plt.semilogx
    else:
        raise(ValueError("Unknown scale: {0}".format(scale)))

    channels = max(map(max, g2s.keys()))+1

    plt.clf()

    for index, val in enumerate(g2s.items(), start=1):
        corr, g2 = val
        c0, c1 = corr

        plt.subplot(channels, channels, index)
        times = list(map(lambda x: mean(x[0]), g2))
        counts = list(map(lambda x: x[1], g2))
   
        plotter(times, counts)
        plt.title(str(corr))
        plt.xlabel(r"$\tau$")
        plt.ylabel(r"$g^{(2)}(\tau)$")

    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot a g2.")

    parser.add_argument("--scale", default="linear", type=str,
                        help="Scale for the time axis display, either "
                             "linear or log.")

    parser.add_argument("files", type=str, nargs="*",
                        help="Filenames containing g2 data to plot.")

    args = parser.parse_args()

    for filename in args.files:
        with open(filename) as stream_in:
            g2s = g2_from_stream(stream_in)

        plot_g2s(g2s, scale=args.scale)
