class TimeWindow(object):
    def __init__(self, lower=None, upper=None,
                 string=None, decode=True):
        if string and decode:
            self.from_string(string)
        else:
            self._string = string
            self.lower = lower
            self.upper = upper

    def __str__(self):
        if not self._string:
            self._string = "{0},{1}".format(lower, upper)
        return(self._string)

    def from_string(self, string):
        self._string = string

        self.lower, self.upper = list(map(self._string.split(",")))

class Intensity(object):
    def __init__(self, time_window=None, counts=None,
                 string=None, decode=True):
        self.time_window = None
        self.counts = None
        
        if string and decode:
            self.from_string(string)
        else:
            self._string = string
            self.time_window = time_window
            self.counts = counts

    def __str__(self):
        if not self._string:
            self._string = "{0},{1}".format(
                self.time_window,
                ",".join(map(str, self.counts)))
                
        return(self._string)

    def from_string(self, string):
        self._string = string
        parts = string.split(",")

        self.time_window = TimeWindow(",".join(parts[:1]))
        self.counts = map(int, parts[2:])
        
        return(self)
