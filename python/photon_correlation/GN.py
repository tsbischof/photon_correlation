import csv
import bz2
import os

from .util import is_cross_correlation

class GN(object):
    def __init__(self, filename=None, stream=None,
                 bins=None, counts=None):
        self._counts = dict()

        if filename is not None:
            self.from_file(filename)
        elif stream is not None:
            self.from_stream(stream)
        elif bins is not None and counts is not None:
            stream = map(lambda x, y: list(x) + [y], zip(*bins), counts)

            self.from_stream(stream)
        else:
            pass

    def __getitem__(self, correlation):
        return(self._counts[correlation])

    def __setitem__(self, correlation, gn):
        self._counts[correlation] = gn

    def __delitem__(self, correlation):
        del(self.counts[correlation])

    def __len__(self):
        return(len(self._counts))

    def __iter__(self):
        return(iter(sorted(self._counts)))

    def from_file(self, filename):
        if not os.path.exists(filename):
            bz2_name = "{}.bz2".format(filename)
            if os.path.exists(bz2_name):
                filename = bz2_name
                
        if filename.endswith("bz2"):
            open_f = lambda x: bz2.open(x, "rt")
        else: 
            open_f = open
            
        with open_f(filename) as stream_in:
            return(self.from_stream(csv.reader(stream_in)))

    def to_file(self, filename):
        with open(filename, "w") as stream_out:
            writer = csv.writer(stream_out)

            for line in self.to_stream():
                writer.writerow(list(map(str, line)))

    def cross_correlations(self):
        return(iter(filter(is_cross_correlation, self)))

    def autocorrelation(self):
        for correlation in filter(lambda x: is_cross_correlation(x),
                                  self._correlations):
            pass

    def unique_peaks(self):
        """
        Report the center/side/etc values associated with the unique types
        of peaks found in the given correlation.
        """
        return(None)
