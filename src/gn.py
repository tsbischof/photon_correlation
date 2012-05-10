#!/usr/bin/env python

import itertools

class Histogram:
     None

class HistogramBin:
     None

if __name__ == "__main__":
     n_channels = 4
     correlation_order = 2
     for channels in itertools.combinations_with_replacement(
          range(n_channels), correlation_order):
          print(channels)

