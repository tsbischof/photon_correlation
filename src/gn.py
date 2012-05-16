#!/usr/bin/env python

import itertools
import csv

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
               range(n_channels),
               order))
                                                    

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
     def __init__(self, ref_channel, bins, counts, correct_parity=True):
          # Modify this to take care of parity
          self.ref_channel = ref_channel
          self._bins = tuple(sorted(bins))
          self.counts = counts
          self.edges = tuple(map(lambda x: x[1:], self._bins))
          self.channels = tuple(map(lambda x: x[0], self._bins))
          self.correlation = tuple(sorted([ref_channel] + list(self.channels)))

          if correct_parity:
               self.correct_parity()

     def correct_parity(self):
          # Based on the given order of the channels, sort them and determine
          # how this changes the sign of the edge. Swapping two channels
          # does nothing, but a swap with the reference channel inverts the
          # swapped dimension.
          pass

     # We should only combine the unique cross-correlations and symmetrize them.
     # Trying to re-evaluate time differences leads to ambiguity of timing
     # or resizing of time bins.

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
     order = 3
     filename = "blargh_{0}.g{1}".format(mode, order)

     correlations = dict()
     for correlation in unique_correlations(n_channels, order):
          correlations[correlation] = Histogram(mode, n_channels, order)

     with open(filename) as stream:
          for histogram_bin in bins_from_stream(stream,
                                                mode,
                                                n_channels,
                                                order):
               correlations[histogram_bin.correlation].add_bin(histogram_bin)

     print(correlations)

     for edges, counts in correlations[(0,0,0)]:
          print(edges, counts)
