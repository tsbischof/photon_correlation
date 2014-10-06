import csv
import os
import itertools

import matplotlib.pyplot as plt

from Lifetime import Lifetime
from util import *

class G1(object):
    def __init__(self, filename=None, run_dir=None, stream=None,
                 bins=None, counts=None):
        self.lifetimes = dict()

        if run_dir is not None:
            self.run_dir = run_dir
            self.filename = "g1"
            
            self.from_file(os.path.join(self.run_dir, self.filename))
        elif filename is not None:
            self.run_dir, self.filename = os.path.split(filename)

            self.from_file(os.path.join(self.run_dir, self.filename))
        elif stream is not None:
            self.run_dir = run_dir
            self.filename = filename

            self.from_stream(stream)
        elif bins is not None and counts is not None:
            stream = map(lambda x, y: list(x) + [y], zip(*bins), counts)

            self.from_stream(stream)
        else:
            raise(ValueError("Must specify a run dir, filename, or data."))
        
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
            counts = int(line[3])

            if not curve in curves:
                curves[curve] = list()

            curves[curve].append(((time_left, time_right), counts))

        for curve, g1 in curves.items():
            times = list(map(lambda x: x[0], g1))
            counts = list(map(lambda x: x[1], g1))
            
            self[curve] = Lifetime(counts, times=times)

        return(self) 

    def from_file(self, filename):
        with open(filename) as stream_in:
            return(self.from_stream(csv.reader(stream_in)))

    def to_stream(self, stream_out):
        writer = csv.writer(stream_out)
        
        for curve in sorted(self.lifetimes):
            for time_bin, counts in self[curve]:
                writer.writerow(list(map(str,
                                         [curve] + list(time_bin) + [counts])))
    
    def to_file(self, filename):
        with open(filename, "w") as stream_out:
            self.to_stream(stream_out)
        
    def __setitem__(self, index, value):
        self.lifetimes[index] = value

    def __getitem__(self, index):
        return(self.lifetimes[index])

    def __iter__(self):
        for curve, lifetime in sorted(self.lifetimes.items()):
            yield(curve, lifetime)

    def combine(self):
        """
        Add together the counts for each channel to produce a single lifetime.
        """
        vals = list(self.lifetimes.values())
        
        return(sum(vals[1:], vals[0]))

    def add_to_axes(self, ax, resolution=None):
        max_xlim = 0

        for curve, lifetime in self:
            if resolution is not None:
                lifetime = lifetime.to_resolution(resolution)
                
            times = list(map(lambda x: x*1e-3, lifetime.times))
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

    
