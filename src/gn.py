#!/usr/bin/env python

import itertools
import csv
import fractions
import math
import functools

T2 = "t2"
T3 = "t3"

##def make_ordered_channels(n_channels, order):
##     for permutation in itertools.product(range(n_channels), repeat=order):
##          sorted_permutation = tuple(sorted(permutation))
##          if permutation != sorted_permutation:
##               # We must deal with it?

def chunks(L, n=2):
     chunk = list()
     for elem in L:
          chunk.append(elem)
          if len(chunk) >= n:
               yield(chunk)
               chunk = list()

def get_bins(bins, mode):
     if mode == T2:
          for chunk in chunks(bins, 3):
               yield((int(chunk[0]),
                      tuple(map(float, chunk[1:3]))))
     elif mode == T3:
          for chunk in chunks(bins, 5):
               yield((int(chunk[0]),
                      tuple(map(float, chunk[1:3])),
                      tuple(map(float, chunk[3:5]))))
     else:
          raise(ValueError("Mode [0} not recognized.".format(mode)))

def unique_correlations(n_channels, order):
     return(
          itertools.combinations_with_replacement(
               itertools.repeat(range(n_channels), order),
               order))

def all_correlations(n_channels, order):
     return(
          itertools.product(
               range(n_channels),
               repeat=order))
                                                    

class Histogram:
     def __init__(self, mode, n_channels, order, correlation=tuple()):
          self.n_channels = n_channels
          self.mode = mode
          self.correlation = correlation
          
          self.bins = dict()

     def __iter__(self):
          return(self.to_stream())

     def add_bin(self, histogram_bin):
          if histogram_bin.edges not in self.bins.keys():
               self.bins[histogram_bin.edges] = 0

          self.bins[histogram_bin.edges] += histogram_bin.counts

     def to_stream(self):
          for key, counts in sorted(self.bins.items()):
               yield(key, counts)

class HistogramBin:
     def __init__(self, ref_channel, bins, counts):
          # Modify this to take care of parity
          self.ref_channel = ref_channel
          self._bins = tuple(bins)
          self.counts = counts
          self.edges = tuple(map(lambda x: x[1:], self._bins))
          self.channels = tuple(map(lambda x: x[0], self._bins))
          self.correlation = tuple([self.ref_channel] + list(self.channels))

     def normalized(self, intensities, resolutions=(1,)):
          if self.counts == 0:
               return(self)
          
          # The numerator has the counts and integration times
          numerator = self.counts

          for channel in self.correlation:
               time_bounds = intensities[channel][0]
               numerator *= time_bounds[1] - time_bounds[0]

          # denominator has resolution and total channel intensity

          # First, the reference channel
          denominator = functools.reduce(lambda x, y: x*y,
                                         resolutions)

          denominator *= intensities[self.ref_channel][1]

          # For the target channels, we must have the total counts and the
          # effective resolution of the bin
          for channel, edges in zip(self.channels, self.edges):
               total_counts = intensities[channel][1]
               denominator *= total_counts

               # T2 has time bin, T3 has pulse and time bins
               effective_bins = map(lambda bounds, res: (float(bounds[0])/res,
                                                         float(bounds[1])/res),
                                    edges, resolutions)
               for bounds in effective_bins:
                    denominator *= int(math.floor(bounds[1])
                                       - math.ceil(bounds[0]))

          try:
               self.counts = fractions.Fraction(numerator, denominator)
          except ZeroDivisionError:
               # A resolution or intensity was 0
               self.counts = 0

          return(self)
               
          
def bins_from_stream(stream, mode, n_channels, order):
     for line in csv.reader(stream):
          ref_channel = int(line.pop(0))
          counts = int(line.pop(-1))

          yield(HistogramBin(ref_channel,
                             get_bins(line, mode),
                             counts))

if __name__ == "__main__":
     mode = T2
     n_channels = 4
     order = 2
     filename = "blargh_{0}.g{1}".format(mode, order)
     intensity_filename = "blargh_{0}.intensity".format(mode)
     resolutions = (1,)

     intensities = dict()
     with open(intensity_filename) as stream:
          line = next(csv.reader(stream))
          for channel, counts in enumerate(line[2:]):
               intensities[channel] = ((int(line[0]), int(line[1])),
                                       int(counts))
               

     correlations = dict()
     for correlation in all_correlations(n_channels, order):
          correlations[correlation] = Histogram(mode, n_channels, order)

     autocorrelation = Histogram(mode, n_channels, order)

     with open(filename) as stream:
          for histogram_bin in bins_from_stream(stream,
                                                mode,
                                                n_channels,
                                                order):
               correlations[histogram_bin.correlation].add_bin(histogram_bin)
               autocorrelation.add_bin(histogram_bin.normalized(intensities,
                                                                resolutions))

     for correlation, histogram in sorted(correlations.items()):
          print(correlation)
          for edges, counts in histogram:
               print(edges, counts)

     print("g({0})".format(order))
     for edges, counts in autocorrelation:
          print(edges, float(counts))
