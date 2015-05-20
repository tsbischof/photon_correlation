import csv
import math
import copy

import numpy

class Counts(object):
    def __init__(self, events, counts):
        self.events = events
        self.counts = counts

    def __add__(self, other):
        return(Counts(self.events + other.events,
                      self.counts + other.counts))

    def normalized(self):
        return(self.counts.astype(float) / self.events)

class IDGN(object):
    def __init__(self, filename):
        self.bins = list()
        self.counts = dict()
        self.window_width = None
        
        self.from_file(filename)

    def __getitem__(self, intensity_bin):
        return(self.counts[intensity_bin])

    def __setitem__(self, intensity_bin, counts):
        self.counts[intensity_bin] = counts

    def __iter__(self):
        return(iter(self.counts))

    def from_file(self, filename):
        with open(filename) as stream_in:
            self.from_stream(stream_in)

    def from_stream(self, stream_in):
        reader = csv.reader(stream_in)

        read_bins = False
        
        for line in reader:
            if not read_bins:
                if not line[0]:
                    self.bins.append(line[4:])
                else:
                    read_bins = True

            if read_bins:
                intensity_bin = tuple(map(int, line[:2]))

                if self.window_width is None:
                    self.window_width = int(line[2])

                events = int(line[3])
                counts = numpy.array(list(map(int, line[4:])))

                self[intensity_bin] = Counts(events, counts)

    def max_intensity(self, threshold=0.7, cast_to_gn=None):
        """
        Add together the bins for intensities above the given threshold,
        which is expressed relative to the highest number of photons seen.
        """
        return(self.intensity_range(threshold, 1, cast_to_gn=cast_to_gn))

    def intensity_range(self, min_val, max_val, cast_to_gn=None):
        """
        Add together the bins for intensities between the given limits,
        expressed as a fraction of the maximum intensity.

        Optionally, yield a GN object formed from the counts.
        """
        maximum = max(map(lambda intensity: intensity[1], self))

        min_intensity = math.floor(min_val*maximum)
        max_intensity = math.ceil(max_val*maximum)

        counts = None

        bins = filter(lambda x: x[0] >= min_intensity and x[1] <= max_intensity,
                      self)
               
        for intensity_bin in bins:
            my_counts = self[intensity_bin]

            if counts is None:
                # If you do not explicitly copy, the result is stored as part
                # of the original counts when doing += 
                counts = copy.deepcopy(my_counts)
            else:
                counts += my_counts

        if counts is None:
            counts = [0 for _ in self.bins]
            
        if cast_to_gn is None:
            return(counts)
        else:
            return(cast_to_gn(counts=counts.counts, bins=self.bins))
