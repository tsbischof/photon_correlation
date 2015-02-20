import numpy

class Gaussian:
    def __init__(self, magnitude, sigma, mu):
        self.sigma = sigma
        self.mu = mu
        self.magnitude = magnitude / numpy.sqrt(numpy.pi*2*self.sigma**2)

    def __call__(self, x):
        return(self.magnitude*numpy.exp(-(x-self.mu)**2/(2*self.sigma**2)))

    def __str__(self):
        return("{0:.2e}*exp(-(t-{1:.2e})^2/(2*{2:.2e}^2))".format(
            self.magnitude, self.mu, self.sigma))
