import subprocess
import math
import os

from picoquant import modes, files, photon

class Limits(object):
    def __init__(self, lower=None, bins=1, upper=None,
                 string=None):
        self._string = string
        
        if string:
            self.from_string(string)
        else:
            self._string = string
            self.lower = lower
            self.bins = bins
            self.upper = upper

    def __str__(self):
        if not self._string:
            self._string = ",".join(
                map(str,
                    [self.lower, self.bins, self.upper]))

        return(self._string)

    def from_string(self):
        self.lower, self.bins, self.upper = map(int, string.split("\n"))
        return(self)

    def __max__(self):
        return(max(map(abs, [self.lower, self.upper])))

    def __min__(self):
        return(min(map(abs, [0, self.lower, self.upper])))

    def __contains__(self, value):
        return(self.lower <= value and value < self.upper)

class TimeBin(object):
     def __init__(self, bounds, resolution=1):
          self.bounds = bounds
          self.resolution = resolution

     def __str__(self):
          return(str(self.bounds))

     def __len__(self):
          return(int(math.ceil(float(self.bounds[1])/self.resolution)
                     - math.floor(float(self.bounds[0])/self.resolution)))
          
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

def chunks(L, n=2):
     chunk = list()
     for elem in L:
          chunk.append(elem)
          if len(chunk) >= n:
               yield(chunk)
               chunk = list()

def get_bins(bins, mode, time_resolution=1, pulse_resolution=1):
     if mode == T2:
          for chunk in chunks(bins, 3):
               channel = int(chunk[0])
               bounds = (float(chunk[1]), float(chunk[2]))
               yield((channel,
                      TimeBin(bounds,
                              resolution=time_resolution)))
     elif mode == T3:
          for chunk in chunks(bins, 5):
              channel = int(chunk[0])
              pulse_bounds = (float(chunk[1]), float(chunk[2]))
              time_bounds = (float(chunk[3]), float(chunk[4]))
              yield(channel,
                    TimeBin(pulse_bounds,
                            resolution=pulse_resolution),
                    TimeBin(time_bounds,
                            resolution=time_resolution))
     else:
          raise(ValueError("Mode {0} not recognized.".format(mode)))

class HistogramBin(object):
    def __init__(self, mode=None, order=None,
                 correlation=None, bins=None,
                 time_resolution=1, pulse_resolution=1,
                 string=None):
        if not mode or not order:
            raise(AttributeError("Must specify mode and order."))

        self.mode = mode
        self.order = order

        self.time_resolution = time_resolution
        self.pulse_resolution = pulse_resolution

        if string:
            self.from_string(string)
        else:
            self.correlation = correlation
            self.bins = bins
            self._string = string

    def from_string(self, string):
        line = ",".split(string)

        counts = float(line[-1])
        if counts.is_integer():
            counts = int(counts)

        correlation = [int(line[0])]

        for my_bin in get_bins(line[1:-1], mode):
            correlation.append(my_bin[0])
            self._bins.append(my_bin)
        
        self._bins = tuple(get_bins(line[1:-1], mode,
                                    time_resolution=self.time_resolution,
                                    pulse_resolution=self.pulse_resolution))
        
        self.ref_channel = int(self.line[0])
        self.counts = int(self.line[-1])
        self._bins = tuple(get_bins(self.line[1:-1], mode))
        return(self)

    def __str__(self):
        if not self._string:
            result = list()

            channels = self.channels()
            bins = self.time_bins()
        
            result.append(str(self.ref_channel))

            for i in range(len(self.channels())):
                result.append(str(channels[i]))
                for dimension in bins[i]:
                    result.append(str(dimension.bounds[0]))
                    result.append(str(dimension.bounds[1]))

            result.append(str(self.counts))

        return(self._string)

    def time_bins(self):
        return(tuple(map(lambda x: x[1:], self._bins)))

    def channels(self):
        return(tuple(map(lambda x: x[0], self._bins)))

    def correlation(self):
        if not self._correlation:
            self._correlation = tuple([self.ref_channel] +
                                      list(map(lambda x: x[0], self._bins)))

        return(self._correlation)

    def __hash__(self):
        return(hash(str(self)))
    
    def volume(self):
        volume = 1
        for my_bin in self.time_bins():
            for bounds in my_bin:
                volume *= len(bounds)

        return(volume)

class Histogram(object):
    def __init__(self, photons,
                 order=1,
                 time_limits=None, pulse_limits=None,
                 time_scale=modes.SCALE_LINEAR, pulse_scale=modes.SCALE_LINEAR,
                 filename=None):
        self.photons = photons

        self.order = order
        self.time_scale = time_scale
        self.time_limits = time_limits

        self.pulse_scale = pulse_scale
        self.pulse_limits = pulse_limits

        self.filename = filename
    
        self._bins = None

        if self.photons.mode == modes.T3 and \
           self.order > 1 and \
           not (self.time_limits and self.pulse_limits):
            raise(ValueError("T3 mode requires pulse and time limits."))
        elif self.photons.mode == modes.T2 and \
             not self.time_limits:
            raise(ValueError("T2 mode requires time limits."))

        if not self.photons.mode in modes.TTTR:
            raise(ValueError("Photon stream must be t2 or t3 type."))

    def bins(self):
        if not self._bins:
            cmd = [files.HISTOGRAM,
                   "--mode", self.photons.mode,
                   "--channels", str(self.photons.channels),
                   "--order", str(self.order)]

            if self.time_limits:
                cmd.extend(["--time", str(self.time_limits),
                            "--time-scale", self.time_scale])

            if self.pulse_limits:
                cmd.extend(["--pulse", str(self.pulse_limits),
                            "--pulse-scale", self.pulse_scale])

            if self.filename:
                cmd.extend(["--file-out", self.filename])

            # Now that we have formed the base command, run the histogram
            if self.filename:
                # Writing to file, we must process the whole thing then pick
                # the data up from that file:
                if not os.path.isfile(self.filename):
                    histogrammer = subprocess.Popen(cmd, stdin=subprocess.PIPE)

                    for my_photon in photon.byte_stream(photons):
                        histogrammer.stdin.write(my_photon)

                    histogrammer.stdin.write(b'\x04')
                                    
                self._bins = map(lambda x: HistogramBin(string=x),
                                 open(self.filename, "r"))
                
            else:
                histogrammer = subprocess.Popen(cmd,
                                                stdin=subprocess.PIPE,
                                                stdout=subprocess.PIPE)

                for my_photon in photon.byte_stream(photons):
                    histogrammer.stdin.write(my_photon)

                histogrammer.stdin.write(b'\x04')

                self._bins = map(lambda x: HistogramBin(string=x.decode()),
                                 histogrammer.stdout.splitlines())

        try:
            yield(next(self._bins))
        except StopIteration:
            self._bins = None
            raise(StopIteration)


if __name__ == "__main__":
    import picoquant

    photons = picoquant.Picoquant("v20.pt3")

    lifetime = Histogram(photons,
                         order=1,
                         time_limits=Limits(0, 100, int(10**6)),
                         filename="v20.pt3.g1")

    for my_bin in lifetime.bins():
        print(my_bin)
        
                 
