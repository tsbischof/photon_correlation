from picoquant import modes

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

##class HistogramBin(object):
##    def __init__(self, correlation=None, limits=None, counts=None, string=None):
##        if string:
##            self.from_string(string)
##        else:
##            self.correlation = correlation
##            self.limits = limits
##            self.counts = counts

class Histogram(object):
    def __init__(self, photons, order=1,
                 time_limits=None, pulse_limits=None,
                 time_scale=modes.SCALE_LINEAR, pulse_scale=modes.SCALE_LINEAR,
                 filename=None):
        self.mode = photons.mode
        self.channels = photons.channels

        self.order = order
        self.time_scale = time_scale
        self.time_limits = time_limits

        self.pulse_scale = pulse_scale
        self.pulse_limits = pulse_limits

        self.filename = filename
    
        self._bins = range(10)

    def bins(self):
        
                 
