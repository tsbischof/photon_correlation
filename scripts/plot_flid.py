#!/usr/bin/env python

import sys
import csv

import matplotlib.pyplot as plt
import numpy

class FLID(object):
    def __init__(self, filename=None):
        self.intensity = list()
        self.arrival_time = list()

        self.counts = list()

        if filename is not None:
            self.from_filename(filename)

    def from_filename(self, filename):
        with open(filename) as stream_in:
            return(self.from_stream(stream_in))

    def from_stream(self, stream_in):
        reader = csv.reader(stream_in)

        time_bins = (next(reader), next(reader))

        for left, right in zip(time_bins[0][2:], time_bins[1][2:]):
            self.arrival_time.append((float(left), float(right)))

        for line in reader:
            self.intensity.append((float(line[0]), float(line[1])))
            self.counts.append(list(map(int, line[2:])))

        return(self)

    def make_figure(self):
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)

        ax.imshow(self.counts, interpolation="none")
        
        return(fig)

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        flid = FLID(filename)
        plt.show(flid.make_figure())
