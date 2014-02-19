import sys
sys.path.append("/home/tsbischof/lib")

import fractions

import photon_correlation as pc

class Correlation(object):
    def __init__(self):
        pass

class BinnedSignal(object):
    def __init__(self, intensity_source, limits, scale=pc.modes.SCALE_LINEAR):
        super(BinnedSignal, self).__init__()

        self.limits = limits
        self.scale = scale
        self.bins = pc.histogram.make_edges(limits, scale)
        self.intensity_source = intensity_source
        self.binned = list()

    def append(self, counts):
        self.intensity.append(counts)

    def pop(self):
        pass

    def __iter__(self):
        return(iter(self))
    
    def __next__(self):
        pass

if __name__ == "__main__":
    correlation = Correlation()
    with open("blargh.intensity") as in_stream:
        signal = list(map(lambda y: y.counts(),
                             map(lambda x: pc.intensity.Intensity(string=x),
                                 in_stream)))
            
    
