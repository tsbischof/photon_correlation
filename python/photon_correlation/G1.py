import csv
import os
import itertools

import matplotlib.pyplot as plt

from .Lifetime import Lifetime
from .GN import GN
from .util import *

class G1(GN):
    def __sub__(self, other):
        dst = G1()

        for channel in self:
            dst[channel] = self[channel] - other[channel]

        return(dst)

    def to_resolution(self, resolution):
        dst = G1()

        for channel in self:
            dst[channel] = self[channel].to_resolution(resolution)

        return(dst)
    
    def from_stream(self, stream_in):
        """
        Given a stream which returns lines in curve, bin_left, bin_right, counts
        format, produce the G1 object.
        """
        curves = dict()
        
        for line in stream_in:
            curve = int(line[0])
            time_left = float(line[1])
            time_right = float(line[2])
            counts = float(line[3])

            if not curve in curves:
                curves[curve] = list()

            curves[curve].append(((time_left, time_right), counts))

        for curve, g1 in curves.items():
            times = list(map(lambda x: x[0], g1))
            counts = list(map(lambda x: x[1], g1))
            
            self[curve] = Lifetime(counts, times=times)

        return(self) 

    def to_stream(self):
        for curve in self:
            lifetime = self[curve]
            for time_bin, counts in lifetime:
                yield([curve] + list(time_bin) + [counts])

    def combine(self):
        """
        Add together the counts for each channel to produce a single lifetime.
        """
        counts = None

        for curve in self:
            if counts is None:
                counts = numpy.array(self[curve].counts)
            else:
                counts += numpy.array(self[curve].counts)
        
        return(Lifetime(counts, times=self[0].times))

    def add_to_axes(self, ax, resolution=None):
        max_xlim = 0

        for curve in self:
            lifetime = self[curve]
            if resolution is not None:
                lifetime = lifetime.to_resolution(resolution)
                
            times = list(map(lambda x: x[0]*1e-3, lifetime.times))
            counts = lifetime.counts
                         
            ax.semilogy(times, counts, label=str(curve))

            my_max = times[final_nonzero(counts)]

            if my_max > max_xlim:
                max_xlim = my_max
                
        ax.set_xlabel("Time/ns")
        ax.set_ylabel("Counts")
        ax.set_xlim((0, max_xlim))
        ax.legend()
        
    def make_figure(self, resolution=None):
        fig = plt.figure()
        self.add_to_axes(fig.add_subplot(111), resolution=resolution)
        return(fig)
