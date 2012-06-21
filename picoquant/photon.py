import subprocess
import csv
import itertools

from picoquant import modes
from picoquant import *

class Photon(object):
    def __init__(self, mode, channel=None, pulse=None, time=None, string=None):
        self.mode = mode
        self._string = string
        
        if string:
            self.from_string(string)
        else:
            self._string = string
            self.channel = channel
            self.pulse = pulse
            self.time = time

    def __str__(self):
        if not self._string:
            if self.mode == modes.T2:
                self._string = ",".join(map(str,
                                            [self.channel,
                                             self.time]))
            elif self.mode == modes.T3:
                self._string = ",".join(map(str,
                                            [self.channel,
                                             self.pulse,
                                             self.time]))
            else:
                raise(ValueError("Mode not recognized: {0}".format(self.mode)))

        return(self._string)

    def from_string(self, string):
        parts = string.split(",")

        if self.mode == modes.T2:
            self.channel = int(parts[0])
            self.time = int(parts[1])
        elif self.mode == modes.T3:
            self.channel = int(parts[0])
            self.pulse = int(parts[1])
            self.time = int(parts[2])
        else:
            raise(ValueError("Mode not recognized: {0}".format(self.mode)))

        return(self)

def windows(start, step):
    lower = start
    while True:
        yield(lower, lower + step)
        lower += step
        

class WindowedStream(object):
    def __init__(self, photons, pulse=None, time=None):
        """Given a stream of photons and some fixed interval over which to split
it (based on pulse or time), this generator yields the photons found in each
window sequentially."""
        # Based on the outline from itertools.groupby
        if pulse:
            self.step = pulse
            self.key = lambda p: p.pulse
        elif time:
            self.step = time
            self.key = lambda p: p.time
        else:
            raise(ValueError("Must specify step size for either pulse or time"))

        self.windows = windows(0, self.step)
        self.lower, self.upper = next(self.windows)
        self.test = lambda p: self.key(p) >= self.lower and \
                    self.key(p) < self.upper

        self.photons = photons
        self._done = False

        self.current_photon = None
        
    def __iter__(self):
        return(self)

    def __next__(self):
        if not self.current_photon:
            self.current_photon = next(self.photons)

        # We cannot use the StopIteration from populate, so check here to see
        # whether the photon stream is dry.
        if self._done:
            raise(StopIteration)
        
        return((self.lower, self.upper), self.populate())

    def populate(self):
        try:
            while self.test(self.current_photon):
                yield(self.current_photon)
                self.current_photon = next(self.photons)
        except StopIteration:
            self._done = True

        self.lower, self.upper = next(self.windows)

        if self._done:
            raise(StopIteration)
            
if __name__ == "__main__":
    import picoquant
    p = picoquant.Picoquant("v20.pt2")

    total = 0
    for index, block in zip(range(100),
                            WindowedStream(p.stream(decode=True),
                                           time=1000000000000)):
        limits, photons = block
        photons = list(photons)
        total += len(photons)
        print(index, limits, len(photons))

    print(total, len(list(p.stream(decode=True))))
