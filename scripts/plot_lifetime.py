#!/usr/bin/env python3

import csv
import sys

import matplotlib.pyplot as plt

import photon_correlation as pc

def plot_lifetime(lifetimes):
    plt.clf()

    for index in lifetimes.keys():
        lifetime = lifetimes[index]
        
        times = list(map(lambda x: x[0][0], lifetime))
        counts = list(map(lambda x: x[1], lifetime))

        if any(counts):
            plt.semilogy(times, counts, label=str(index))
        else:
            pass

    plt.legend()
    plt.show()

def lifetimes_from_file(stream):
    result = dict()
    
    for line in csv.reader(stream):
        index = int(line[0])
        bin_left = float(line[1])
        bin_right = float(line[2])
        counts = int(line[3])

        if not index in result.keys():
            result[index] = list()

        result[index].append(((bin_left, bin_right), counts))

    return(result)

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)
        g1 = pc.G1(filename=filename)
        g1.add_to_axes(ax)
        plt.show(fig)

#        with open(filename) as stream:
#            lifetimes = lifetimes_from_file(stream)
#
#            plot_lifetime(lifetimes)

