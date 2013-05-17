#!/usr/bin/env python

import csv
import sys
import math
import argparse

import matplotlib.pyplot as plt

def mean(L):
    return(float(sum(L))/len(L))

def g2_from_stream(stream):
    result = dict()
    for channel_0, channel_1, p_left, p_right, t_left, t_right, counts in \
        csv.reader(stream):
        key = (int(channel_0), int(channel_1))
        if key not in result.keys():
            result[key] = list()

        result[key].append(((float(p_left), float(p_right)),
                            (float(t_left), float(t_right)),
                            float(counts)))

    return(result)

def plot_g2s(g2s):
    channels = max(map(max, g2s.keys()))+1

    plt.clf()

    corr = (0, 1)
    g2 = g2s[corr]

    pulse_bins = set(map(lambda x: x[0], g2))

    for pulse_bin in sorted(pulse_bins):
        my_g2 = list(filter(lambda x: x[0] == pulse_bin, g2))
        
        times = list(map(lambda x: mean(x[1]), my_g2))
        counts = list(map(lambda x: x[2], my_g2))

        if not any(counts):
            pass
        else:
            plt.semilogy(times, counts, label=str(pulse_bin))

    plt.xlabel(r"$\tau$")
    plt.ylabel(r"$g^{(2)}(\tau)$")
    plt.legend()

    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot a g2 of t3 data.")

    parser.add_argument("files", type=str, nargs="*",
                        help="Filenames containing g2 data to plot.")

    args = parser.parse_args()

    for filename in args.files:
        with open(filename) as stream_in:
            g2s = g2_from_stream(stream_in)

        plot_g2s(g2s)
