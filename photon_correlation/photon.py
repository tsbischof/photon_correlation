import subprocess
import csv
import itertools

import photon_correlation as pc

class Photon(object):
    def __init__(self, mode, channel=None, pulse=None, time=None,
                 string=None, decode=True):
        self.mode = mode
        
        if string and decode:
            self.from_string(string)
        else:
            self._string = string
            self.channel = channel
            self.pulse = pulse
            self.time = time

    def __str__(self):
        if not self._string:
            if self.mode == pc.modes.T2:
                self._string = ",".join(map(str,
                                            [self.channel,
                                             self.time]))
            elif self.mode == pc.modes.T3:
                self._string = ",".join(map(str,
                                            [self.channel,
                                             self.pulse,
                                             self.time]))
            else:
                raise(ValueError("Mode not recognized: {0}".format(self.mode)))

        return(self._string)

    def from_string(self, string):
        self._string = string
        parts = string.split(",")

        if self.mode == pc.modes.T2:
            self.channel = int(parts[0])
            self.time = int(parts[1])
        elif self.mode == pc.modes.T3:
            self.channel = int(parts[0])
            self.pulse = int(parts[1])
            self.time = int(parts[2])
        else:
            raise(ValueError("Mode not recognized: {0}".format(self.mode)))

        return(self)

def windows(start, step):
    lower = start
    while True:
        yield(pc.histogram.Limits(lower=lower,
                                  upper=lower+step))
        lower += step

class PhotonStream(object):
    def __init__(self, photons=list(), mode=None, order=None, channels=None):
        self.photons = photons
        self.mode = mode
        self.channels = channels
        self.order = order
        
    def __iter__(self):
        return(self.photons)

class WindowedStream:
    def __init__(self, photons, pulse_bin=None, time_bin=None):
        """Given a stream of photons and some fixed interval over which to split
it (based on pulse or time), this generator yields the photons found in each
window sequentially."""        
        # Based on the outline from itertools.groupby
        if pulse_bin:
            self.step = pulse_bin
            self.key = lambda p: p.pulse
        elif time_bin:
            self.step = time_bin
            self.key = lambda p: p.time
        else:
            raise(ValueError("Must specify step size for either pulse or time"))

        self.windows = windows(0, self.step)
        self.window = next(self.windows)
        self.test = lambda p: self.key(p) in self.window

        self.photons = photons
        self.mode = self.photons.mode
        self.channels = self.photons.channels
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
        
        return(self.window, PhotonStream(photons=self.populate(),
                                         channels=self.photons.channels,
                                         mode=self.photons.mode))

    def populate(self):
        try:
            while self.test(self.current_photon):
                yield(self.current_photon)
                self.current_photon = next(self.photons)
        except StopIteration:
            self._done = True

        self.window = next(self.windows)

        if self._done:
            raise(StopIteration)

def byte_stream(photons):
    for photon in photons:
        if isinstance(photon, bytes):
            yield(photon)
        else:
            yield(str(photon).encode())
            
        yield("\n".encode())
        
if __name__ == "__main__":
    p = pc.Picoquant("../sample_data/picoharp/v20.pt2", decode=True)

##    for photon in p:
##        print(photon)

    total = 0
    for index, block in zip(range(100),
                            WindowedStream(p,
                                           time_bin=int(10**12))):
        limits, photons = block
        n_photons = len(list(photons))
        total += n_photons
        print(index, limits, n_photons)

    print(total, len(list(p)))
