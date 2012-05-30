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
          self.order = order
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

     def normalized(self, intensities, time_resolution=1, pulse_resolution=1):
          for histogram_bin in self.bins:
               if histogram_bin.counts == 0:
                    yield(histogram_bin)

               try:
                    # Each term starts with the raw counts.
                    counts = fractions.Fraction(self.counts, 1)

                    # To normalize the counts, we must account for the
                    # average
                    # number of photons per unit phase space, and the volume
                    # of phase space counted:
                    #                 1
                    # ---------------------------------------
                    # prod(average intensity) prod(bin width)
                    if self.mode == T2:
                         integration_time = intensities[self.ref_channel].span(
                              time_resolution)

                         # The average intensity for a channel appears for
                         # every time the 
                         average_intensities = map(
                              lambda x: fractions.Fraction(
                                   x.counts, x.span(time_resolution)),
                              intensities)
                         
                         counts /= functools.reduce(lambda x, y: x*y,
                                                    average_intensities)

                         # The reference channel draws an integration
                         # time's worth of uncertainty
                         counts /= integration_time

                         # The others get the width of the histogram bin.
                         for channel, time_bin in zip(self.channels,
                                                      self.time_bins):
                              counts /= time_bin[0].span(time_resolution)
               except ZeroDivisionError:
                    # Some resolution or intensity was 0
                    counts = 0
               except AttributeError as error:
                    raise(AttributeError(error))

               yield(HistogramBin(histogram_bin.bounds, counts))

class HistogramBin(object):
     def __init__(self, mode, ref_channel, bins, counts):
          self.mode = mode
          self.ref_channel = ref_channel
          self._bins = tuple(bins)
          self.counts = counts
          self.time_bins = tuple(map(lambda x: x[1:], self._bins))
          self.channels = tuple(map(lambda x: x[0], self._bins))
          self.correlation = tuple([self.ref_channel] + list(self.channels))
          
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

def autocorrelation_from_cross(correlations, intensities):
     pass

if __name__ == "__main__":
     mode = T2
     n_channels = 4
     order = 3
     name_base = "blargh"
     filename = "{0}_{1}.g{2}".format(name_base, mode, order)
     intensity_filename = "{0}_{1}.intensity".format(name_base, mode)
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

     for correlation, histogram in sorted(correlations.items()):
          print(correlation)
          for time_bin in histogram.normalized(intensities,
                                               time_resolution,
                                               pulse_resolution):
               print(time_bin.bounds, time_bin.counts)
          break
