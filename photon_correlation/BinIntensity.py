import csv
import operator

class BinIntensityBin(object):
    def __init__(self, limits, counts):
        self.limits = limits
        self.counts = counts

    def time(self):
        return(self.limits[0])

class BinIntensity(object):
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
            for line in csv.reader(stream_in):
                bin_lower = float(line[0])
                bin_upper = float(line[1])
                counts = tuple(map(int, line[2:]))

                self.bins.append(BinIntensityBin((bin_lower, bin_upper), counts))

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
        
        for channel, counts in self.counts():
            plt.plot(times, counts, label=str(channel))

        plt.xlabel("t/s")
        plt.ylabel("Counts")
        plt.legend()
        
        plt.show()
        
if __name__ == "__main__":
    intensity = BinIntensity().from_file("blargh.bin_intensity")
    intensity.plot()
