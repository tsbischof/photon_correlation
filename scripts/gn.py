#!/usr/bin/env python

import itertools
import csv
import fractions
import math
import functools
import collections
import optparse

picoquant = "picoquant"
intensity = "intensity"
correlate = "correlate"
histogram = "histogram"

t2 = "t2"
t3 = "t3"

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
     if mode == t2:
          for chunk in chunks(bins, 3):
               channel = int(chunk[0])
               bounds = (float(chunk[1]), float(chunk[2]))
               yield((channel, TimeBin(bounds)))
     elif mode == t3:
          for chunk in chunks(bins, 5):
               yield((int(chunk[0]),
                      tuple(map(lambda x: TimeBin(x),
                                map(float, chunk[1:3]))),
                      tuple(map(lambda x: TimeBin(x),
                                map(float, chunk[3:5])))))
     else:
          raise(ValueError("Mode [0} not recognized.".format(mode)))

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

                    if self.mode == t2:
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
    parser = optparse.OptionParser()
    parser.add_option("-m", "--mode", dest="mode",
                      help="Mode to interpret the data as. By default, "
                          "the program will attempt to guess the mode.",
                      action="store")
    parser.add_option("-w", "--intensity-bin-width", dest="intensity_bin_width",
                      help="Set the bin width for an intensity run, in ms (t2) "
                          "or pulses (t3).",
                      action="store", type=float)
    parser.add_option("-n", "--number", dest="number",
                       help="Number of entries to process. By default, all "
                           "entries are processed.",
                       action="store", type=int)
    parser.add_option("-c", "--channels", dest="channels",
                      help="Number of channels in the data. The default is "
                          "to guess the number from the file type.",
                      action="store", type=int)
    parser.add_option("-g", "--order", dest="order",
                      help="Order of the correlation to run. The default is 2.",
                      action="store", type=int)
    parser.add_option("-v", "--verbose", dest="verbose",
                      help="Print debug-level information.",
                      action="store_true", default=False)
    parser.add_option("-q", "--quiet", dest="quiet",
                      help="Suppress all non-vital messages.",
                      action="store_true", default=False)
    parser.add_option("-d", "--time", dest="time_limits",
                      help="Specify the time limits for a histogram run, "
                          "as lower,bins,upper.",
                      action="store")
    parser.add_option("-e", "--pulse", dest="pulse_limits",
                      help="Specify the pulse limits for a histogram run.",
                      action="store")
    parser.add_option("-D", "--time-scale", dest="time_scale",
                      help="Scale for the time axis of a histogram run. Can "
                      "be linear, log, or log-zero (includes zero-time bin)",
                      default="linear", action="store")
    parser.add_option("-p", "--print-every", dest="print_every",
                      help="Print the record number whenever it is divisible "
                      "by this number. By default, nothing is printed.",
                      default=0, action="store", type=int)

    
     mode = t2
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
