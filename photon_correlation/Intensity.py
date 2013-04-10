import csv
import operator

class IntensityBin(object):
    def __init__(self, limits, counts):
        self.limits = limits
        self.counts = counts

    def time(self):
        return(self.limits[0])

    def normalized(self):
        return(tuple(map(lambda x: x*1e12/\
                         float(self.limits[1] - self.limits[0]),
                         self.counts)))

class Intensity(object):
    def __init__(self):
        self.channels = 0
        self.bins = list()

    def __getitem__(self, index):
        if index >= len(self):
            raise(IndexError)
        else:
            return(self.bins[index])

    def __len__(self):
        return(len(self.bins))

    def from_file(self, filename):
        with open(filename) as stream_in:
            return(self.from_stream(stream_in))

    def from_stream(self, stream_in):
        for line in csv.reader(stream_in):
            bin_lower = int(line[0])
            bin_upper = int(line[1])
            counts = tuple(map(int, line[2:]))

            self.bins.append(IntensityBin((bin_lower, bin_upper), counts))

            if len(counts) > self.channels:
                self.channels = len(counts)

        return(self)

    def times(self):
        return(map(lambda x: x.time(), self.bins))

    def counts(self):
        for channel in range(self.channels):
            yield((channel,
                   map(lambda x: x.counts[channel], self.bins)))

    def normalized(self):
        for channel in range(self.channels):
            yield((channel,
                   map(lambda x: x.normalized()[channel], self.bins)))

    def plot(self):
        import matplotlib.pyplot as plt

        plt.clf()
        times = tuple(map(lambda x: float(x)*1e-12, self.times()))
        
        for channel, counts in self.normalized():
            plt.plot(times, counts, label=str(channel))

        plt.xlabel("t/s")
        plt.ylabel("Counts/Hz")
        plt.legend()
        
        plt.show()

if __name__ == "__main__":
    intensity = Intensity().from_file("blargh.intensity")
    intensity.plot()
