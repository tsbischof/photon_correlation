#!/usr/bin/env python

import itertools
import csv
import fractions
import math
import functools
import collections

T2 = "t2"
T3 = "t3"

##def make_ordered_channels(n_channels, order):
##     for permutation in itertools.product(range(n_channels), repeat=order):
##          sorted_permutation = tuple(sorted(permutation))
##          if permutation != sorted_permutation:
##               # We must deal with it?

def flatten(LoL):
     for elem in LoL:
          if isinstance(elem, collections.Iterable):
               for minor in flatten(elem):
                    yield(minor)
          else:
               yield(elem)

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
               channel = int(chunk[0])
               bounds = (float(chunk[1]), float(chunk[2]))
               yield((channel, TimeBin(bounds)))
     elif mode == T3:
          for chunk in chunks(bins, 5):
               yield((int(chunk[0]),
                      tuple(map(lambda x: TimeBin(x),
                                map(float, chunk[1:3]))),
                      tuple(map(lambda x: TimeBin(x),
                                map(float, chunk[3:5])))))
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
                                                    

class Histogram(object):
     def __init__(self, mode, n_channels, order, correlation=tuple()):
          self.n_channels = n_channels
          self.mode = mode
          self.correlation = correlation
          
          self.bins = dict()

     def __iter__(self):
          return(self.to_stream())

     def add_bin(self, histogram_bin):
          key = histogram_bin.time_bins
          
          if key not in self.bins.keys():
               self.bins[key] = 0

          self.bins[key] += histogram_bin.counts

     def to_stream(self):
          for key, counts in sorted(self.bins.items()):
               bins = list()
               for dimension in key:
                    for time_bin in dimension:
                         bins.append(time_bin.bounds)
               yield((tuple(bins), counts))

class HistogramBin(object):
     def __init__(self, mode, ref_channel, bins, counts):
          self.mode = mode
          self.ref_channel = ref_channel
          self._bins = tuple(bins)
          self.counts = counts
          self.time_bins = tuple(map(lambda x: x[1:], self._bins))
          self.channels = tuple(map(lambda x: x[0], self._bins))
          self.correlation = tuple([self.ref_channel] + list(self.channels))

     def normalized(self, intensities, time_resolution, pulse_resolution=1):
          if self.counts == 0:
               return(self)

          try:
               # Each term starts with the raw counts.
               counts = fractions.Fraction(self.counts, 1)

               # To normalize the counts, we must account for the average
               # number of photons per unit phase space, and the volume
               # of phase space counted.
               #
               # For the reference channel, we have:
               #          (integration time)
               # -------------------------------------------------------------
               # (integration time-largest time delay)(counts in the channel)
               # For each dimension of time.
               #
               # For the other channels, we have the same thing, except the
               # resolution now includes all of the possible times in the bin,
               # based on its width and the resolution of the measurement:
               #          (integration time)
               # -----------------------------------
               # (bin width)(counts in the channel)
               if self.mode == T2:
                    integration_time = intensities[self.ref_channel].span(
                         time_resolution)
                    
                    counts *= integration_time               
                    counts /= intensities[self.ref_channel].counts
                    counts /= (integration_time -
                               max(
                                    map(abs,
                                        flatten(
                                             map(lambda x:
                                                 map(lambda y: y.bounds,
                                                     x),
                                                 self.time_bins)))))

                    for channel, time_bin in zip(self.channels, self.time_bins):
##                         print(float(counts),
##                               intensities[channel].span(time_resolution),
##                               intensities[channel].counts,
##                               time_bin[0].span(time_resolution))
                         counts *= intensities[channel].span(time_resolution)
                         counts /= intensities[channel].counts
                         counts /= time_bin[0].span(time_resolution)
                         
##               print(float(counts))

          except ZeroDivisionError:
               # Some resolution or intensity was 0
               counts = 0

          self.counts = counts
          return(self)

class TimeBin(object):
     def __init__(self, bounds, counts=0):
          self.bounds = bounds
          self.counts = counts

     def __str__(self):
          return(str(self.bounds))

     def span(self, resolution=None):
          if resolution:
               return(int(math.ceil(float(self.bounds[1])/resolution)
                         - math.floor(float(self.bounds[0])/resolution)))
          else:
               return(self.bounds[1] - self.bounds[0])

     def __lt__(self, other):
          return(self.bounds < other.bounds)

     def __gt__(self, other):
          return(self.bounds > other.bounds)

     def __eq__(self, other):
          return(self.bounds == other.bounds)

     def __le__(self, other):
          return(self.bounds <= other.bounds)

     def __ge__(self, other):
          return(self.bounds >= other.bounds)

     def __hash__(self):
          return(self.bounds.__hash__())
               
          
def bins_from_stream(stream, mode, n_channels, order):
     for line in csv.reader(stream):
          ref_channel = int(line.pop(0))
          counts = int(line.pop(-1))

          yield(HistogramBin(mode,
                             ref_channel,
                             get_bins(line, mode),
                             counts))

if __name__ == "__main__":
     mode = T2
     n_channels = 4
     order = 2
     filename = "blargh_{0}.g{1}".format(mode, order)
     intensity_filename = "blargh_{0}.intensity".format(mode)
     time_resolution = 1
     pulse_resolution = 1

     intensities = dict()
     with open(intensity_filename) as stream:
          line = next(csv.reader(stream))
          for channel, counts in enumerate(line[2:]):
               intensities[channel] = TimeBin(tuple(map(int, line[0:2])),
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
               autocorrelation.add_bin(
                    histogram_bin.normalized(intensities,
                                             time_resolution,
                                             pulse_resolution))
##               break

##     for correlation, histogram in sorted(correlations.items()):
##          print(correlation)
##          for edges, counts in histogram:
##               print(edges, counts)
##
     print("g({0})".format(order))
     for time_bins, counts in autocorrelation:
          print(time_bins, float(counts))
