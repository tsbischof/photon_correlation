import subprocess

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
            self._string = ",".join(map(str,
                                        [self.lower,
                                         self.bins,
                                         self.upper]))

        return(self._string)

    def from_string(self, string):
        self.lower, self.bins, self.upper = map(int, string.split("\n"))
        return(self)

    def __max__(self):
        return(max(map(abs, [self.lower, self.upper])))

    def __min__(self):
        return(min(map(abs, [0, self.lower, self.upper])))

    def __contains__(self, value):
        return(self.lower <= value and value < self.upper)

class Histogram(object):
    def __init__(self, photons, order=1,
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

        self.run()

    def run(self):
        cmd = [files.HISTOGRAM,
               "--mode", self.photons.mode,
               "--channels", self.photons.channels]

        if self.time_limits:
            cmd.extend(["--time", str(self.time_limits),
                        "--time-scale", self.time_scale])

        if self.pulse_limits:
            cmd.extend(["--pulse", str(self.pulse_limits),
                        "--pulse-scale", self.pulse_scale])

        if self.filename:
            cmd.extend(["--file-out", self.filename])
            histogrammer = subprocess.Popen(
                cmd,
                stdin=subprocess.PIPE)
            histogrammer.communicate(self.photons.stream())
            
##        subprocess.Popen([files.HISTOGRAM,

    def bins(self):
        if not self._bins:
            pass

        return(self._bins)
            
        
                 
