import numpy

class Exponential:
    def __init__(self, magnitude, rate):
        self.magnitude = magnitude
        self.rate = rate

    def __call__(self, x):
        return(self.magnitude*numpy.exp(-self.rate*numpy.array(x)))

    def __str__(self):
        return("{0:.2e}*exp(-{1:.2e}*t)".format(self.magnitude, self.rate))

    @property
    def area(self):
        return(self.magnitude/self.rate)


class MultiExponential:
    def __init__(self, params):
        self.exponentials = [Exponential(magnitude, rate) for magnitude, rate \
                             in zip(params[::2], params[1::2])]

    def __iter__(self):
        return(iter(self.exponentials))
    
    def __call__(self, x):
        return(sum(map(lambda f: f(x), self)))

    def __str__(self):
        return(" + ".join(map(str, self)))

    def __getitem__(self, index):
        return(self.exponentials[index])

    def __setitem__(self, index, value):
        self.exponentials[index] = value

    def sorted_by_area(self):
        return(list(sorted(self.exponentials, key=lambda x: x.area)))

    def sorted_by_rate(self):
        return(list(sorted(self.exponentials, key=lambda x: x.rate)))

    def areas(self):
        return(map(lambda x: x.area, self.exponentials))

    def rates(self):
        return(map(lambda x: x.rate, self.exponentials))

