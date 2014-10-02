class Limits(object):
    def __init__(self, lower, upper, n_bins=1):
        self.lower = lower
        self.upper = upper
        self.n_bins = n_bins

    def __str__(self):
        return("{},{},{}".format(self.lower, self.n_bins, self.upper))
