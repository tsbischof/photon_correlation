#!/usr/bin/env python

import sys
import csv

import matplotlib.pyplot as plt
import numpy

def force_aspect(ax,aspect=1):
    im = ax.get_images()
    extent =  im[0].get_extent()
    ax.set_aspect(abs((extent[1]-extent[0])/(extent[3]-extent[2]))/aspect)

class FLID(object):
    def __init__(self, filename=None):
        self.intensity = list()
        self.arrival_time = list()
        self.events = list()

        self.counts = list()

        if filename is not None:
            self.from_filename(filename)

    def from_filename(self, filename):
        with open(filename) as stream_in:
            return(self.from_stream(stream_in))

    def from_stream(self, stream_in):
        reader = csv.reader(stream_in)

        time_bins = (next(reader), next(reader))

        for left, right in zip(time_bins[0][3:], time_bins[1][3:]):
            self.arrival_time.append((float(left), float(right)))

        for line in reader:
            self.intensity.append((float(line[0]), float(line[1])))
            self.events.append(int(line[2]))
            self.counts.append(list(map(int, line[3:])))

        return(self)

    def make_figure(self):
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)

        ax.imshow(self.counts,
                  interpolation="none",
                  origin="lower",
                  extent=[self.arrival_time[0][0],
                          self.arrival_time[-1][1],
                          self.intensity[0][0],
                          self.intensity[-1][1]])
        force_aspect(ax)
        ax.set_xlabel("Time/ps")
        ax.set_ylabel("Counts per bin")
        fig.tight_layout()
        
        return(fig)
