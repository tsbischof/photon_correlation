#!/usr/bin/env python

import csv
import sys
import math

import matplotlib.pyplot as plt

def g2_from_stream(stream):
    for channel_0, channel_1, bin_left, bin_right, counts in csv.reader(stream):
        yield((int(channel_0), int(channel_1),
               float(bin_left), float(bin_right),
               float(counts)))

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        with open(filename) as stream:
            g2 = list(g2_from_stream(stream))

            correlations = set(map(lambda x: (x[0], x[1]),
                                   g2))
            channels = int(math.sqrt(len(correlations)))

            plt.clf()
            for index, correlation in enumerate(sorted(correlations)):
                plt.subplot(channels, channels,index)
                my_correlation = list(
                    filter(lambda x: (x[0], x[1]) == correlation,
                           g2))
                my_correlation.sort()

                plt.plot(map(lambda x: (x[2]+x[3])/2, my_correlation),
                            map(lambda x: x[4], my_correlation))
                plt.title(str(correlation))
                plt.xlabel("t")
                plt.ylabel("g2")
            plt.show()
