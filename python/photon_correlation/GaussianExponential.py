import functools
import math

import numpy

from .Exponential import *

class GaussianExponential:
    """
    Implements the convolution of a Gaussian with a multiexponential.
    The Gaussian is assumed to be centered at the origin, and the exponential
    is assumed to be modulated by a Heaviside function (also starting at the
    origin).
    """
    def __init__(self, gaussian_magnitude, gaussian_sigma,
                 exponential_parameters):
        self.mx = MultiExponential(exponential_parameters)
        self.gaussian_magnitude = gaussian_magnitude
        self.gaussian_sigma = gaussian_sigma

    def __call__(self, tau):
        # k/2*(exp^(k^2*sigma^2/2 - k*tau))
        # *(1+erf((tau-k*sigma^2)/(sigma*sqrt(2))
        return(
            functools.reduce(
                lambda x, y: x+y,
                map(
                    lambda exponential: \
                    self.gaussian_magnitude*exponential.magnitude*\
                    exponential.rate/2*\
                    (1+numpy.array(
                        list(map(lambda t: math.erf(\
                            (t-exponential.rate*self.gaussian_sigma**2)/\
                            (self.gaussian_sigma*math.sqrt(2))), tau))))*\
                    (numpy.exp(-exponential.rate*tau+\
                               exponential.rate**2*self.gaussian_sigma**2/2)),
                    self.mx)))
