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
                counts = numpy.array(map(int, line[4:]))

                self[intensity_bin] = Counts(events, counts)

    def max_intensity(self, threshold=0.7):
        """
        Add together the bins for intensities above the given threshold,
        which is expressed relative to the highest number of photons seen.
        """
        return(self.intensity_range(threshold, 1))

    def intensity_range(self, min_val, max_val):
        """
        Add together the bins for intensities between the given limits,
        expressed as a fraction of the maximum intensity.
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

        return(counts)

if __name__ == "__main__":
    import matplotlib.pyplot as plt
    
    idgn = IDGN("/home/tsbischof/Documents/data/"
                "microscopy/analysis/triexciton/"
                "2014-09-04_oc2014-04-08/"
                "oc2014-04-08_1e-5_dot_009_250nW_000.ht3.idgn.run/g3.5000")

    events, counts = idgn.max_intensity()

    print(events)
    plt.figure()
    plt.plot(counts)
    plt.show()
