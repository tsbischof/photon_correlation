from photon_correlation import histogram
import photon_correlation

class Interactive(object):
    def __init__(self, filename):
        """Interactive data from a Picoquant device, reported as a
set of histgograms. Filename points to an interactive-type
file, such as phd or thd."""
        self.bins = list()
        self._curves = set()

        for my_bin in photon_correlation.Picoquant(filename):
            self.bins.append(my_bin)
            self._curves.add(self.bins[-1].curve)

    def curve(self, index=0):
        """Return all bins corresponding to the curve of the given index."""
        return(filter(lambda x: x.curve == index,
                      self.bins))

    def curves(self):
        """Yields each curve in the data."""
        for curve_index in sorted(self._curves):
            yield(self.curve(curve_index))

class InteractiveBin(object):
    def __init__(self, curve=None, limits=None, counts=None, string=None):
        """Repesents a single bin from an interactive curve, as defined by
the curve index, bin limits, and the number of counts in that bin.

Passing a string instead of the other parameters is equivalent
to calling InteractiveBin().from_string(string)."""
        self._string = string
        
        if string:
            self.from_string(string)
        else:
            self._string = string
            self.curve = curve
            self.limits = limits
            self.counts = counts

    def from_string(self, string):
        """Parses the output of a picoquant stream to obtain the bin
parameters:
        curve index (int), bin limit lower (float), ...
            bin limit upper (float), counts (int)"""
        raw_values = string.split(",")

        self.curve = int(raw_values[0])
        self.limits = histogram.Limits(lower=float(raw_values[1]),
                                       upper=float(raw_values[2]))
        self.counts = int(raw_values[3])

        return(self)

    def __str__(self):
        if not self._string:
            self._string = "{0},{1},{2},{3}".format(self.curve,
                                                    self.limits.lower,
                                                    self.limits.upper,
                                                    self.counts)

        return(self._string)

if __name__ == "__main__":
    data = Interactive("v20.phd")
    for curve in data.curves():
        print(curve)
