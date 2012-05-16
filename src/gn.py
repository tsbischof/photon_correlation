#!/usr/bin/env python

import itertools
import csv

T2 = "t2"
T3 = "t3"

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
          
          self.bins = list()

     def from_stream(self, stream):
          for line in csv.reader(stream):
               ref_channel = int(line.pop(0))
               counts = int(line.pop(-1))

               self.bins.append(
                    HistogramBin(ref_channel,
                                 get_bins(line, self.mode),
                                 counts))

class HistogramBin:
     def __init__(self, ref_channel, bins, counts):
          self.ref_channel = ref_channel
          self._bins = tuple(sorted(bins))
          self.counts = counts
          self.edges = tuple(map(lambda x: x[1:], self._bins))
          self.channels = tuple(map(lambda x: x[0], self._bins))
          self.correlation = tuple(sorted([ref_channel] + list(self.channels)))
          
if __name__ == "__main__":
     n_channels = 4
     order = 3

     histograms = Histogram(T2, n_channels, order)
     with open("blargh_t2.g3") as data:
          histograms.from_stream(data)

     correlations = dict()
     for correlation in unique_correlations(n_channels, order):
          correlations[correlation] = Histogram(T2, n_channels, order)

     for my_bin in histograms.bins:
          correlations[correlation].add(my_bin)
