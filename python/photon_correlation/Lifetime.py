import bisect
import math

import numpy
import scipy.optimize

from util import *

max_val_default = 0.95
min_val_default = 0.1

class Lifetime(object):
    def __init__(self, counts, times=None, resolution=None):
        """
        If times are specified, they are used directly. If not, they are
        generated from the resolution of the measurement.
        """
        self.times = times
        
        if self.times is None:
            self.times = [(i*self.resolution,
                           (i+1)*self.resolution)
                          for i in range(len(counts))]

        if resolution is None:
            self.resolution = self.times[0][1] - self.times[0][0]
        
        self.counts = counts

        if len(self.times) != len(self.counts):
            raise(ValueError("Times and counts must be the same length."))

    def __add__(self, other):
        if not self.times == other.times:
            raise(ValueError("Attempting to add two unlike lifetimes."))

        counts = list()

        for mine, theirs in zip(self.counts, other.counts):
            counts.append(mine + theirs)
        
        return(Lifetime(counts, times=self.times))

    def __iter__(self):
        return(iter(zip(self.times, self.counts)))

    def time_bins(self):
        return(map(mean, self.times))

    def normalized(self, key=max):
        """
        Return the normalized lifetime, based on the given function.
        """
        return(Lifetime(normalize(self.counts, key=key), times=self.times))

    def to_resolution(self, resolution=1024):
        """
        Perform the rebinning necessary to achieve the desired resolution,
        and return a new lifetime object.
        """
        binning = int(resolution / self.resolution)
        
        return(self.rebin(n=binning))
    
    def rebin(self, n=2):
        """
        Collect every n bins and add them together. Return the result as a new
        lifetime object.
        """
        times = list(zip(smooth(map(lambda x: x[0], self.times), n=n),
                         smooth(map(lambda x: x[1], self.times), n=n)))
        counts = rebin(self.counts, n=n)

        return(Lifetime(counts, times=times))

    def range(self, lower, upper):
        """
        Return the counts associated with the given range of times.
        """
        index_lower = bisect.bisect_left(self.time_bins(), mean(lower))
        index_upper = bisect.bisect_left(self.time_bins(), mean(upper))

        return(Lifetime(self.counts[index_lower:index_upper],
                        times=self.times[index_lower:index_upper]))

    def origin(self):
        """
        Return the index at which the curve is maximal.
        """
        return(self.counts.index(max(self.counts)))

    def fit_data(self, min_val=min_val_default, max_val=max_val_default):
        """
        Find the range associated with the given threshold of values to
        perform a fit. Use the data which vall after the counts have decayed
        to max_val of their maximum value, but before they reach min_val of
        their maximum value.
        """
        # only look after the maximum
        origin = self.origin()
        
        max_counts = max(self.counts)

        counts_lower = min(self.counts[origin:],
                           key=lambda x: abs(x - max_counts*min_val))
        counts_upper = min(self.counts[origin:],
                           key=lambda x: abs(x - max_counts*max_val))

        index_left = last_index(self.counts, counts_upper)
        index_right = last_index(self.counts, counts_lower)

        return(self.range(self.times[index_left],
                          self.times[index_right]))
        
    def exponential_fit(self,
                        min_val=min_val_default,
                        max_val=max_val_default,
                        order=1,
                        error=False):
        """
        Perform an exponential fit of the counts.
        """
        fit_times = list()
        fit_counts = list()

        fit_data = self.fit_data(min_val, max_val)

        for fit_time, fit_count in zip(fit_data.time_bins(), fit_data.counts):
            if fit_count != 0:
                fit_times.append(fit_time)
                fit_counts.append(fit_count)

        fit_times = numpy.array(fit_times)
        fit_counts = numpy.log(fit_counts)

        fit = numpy.polyfit(fit_times, fit_counts, order)
        func = lambda x: numpy.exp(numpy.poly1d(fit)(x))
        
        if error:
            try:
                b, a = fit
                n = len(fit_times)

                meanx = mean(fit_times)
                meany = mean(fit_counts)

                ssxx = sum(fit_times * fit_times) - n*meanx**2
                ssyy = sum(fit_counts * fit_counts) - n*meany**2
                ssxy = sum(fit_times*fit_counts) - n*meanx*meany

                s = math.sqrt((ssyy - ssxy**2/ssxx**2)/(n-2))
                intercept_error = s*math.sqrt(1.0/n + meanx**2/ssxx)
                slope_error = s/math.sqrt(ssxx)
            
                return(fit, func, 1/fit[0]*(slope_error/fit[0]))
            except ZeroDivisionError:
                return(fit, func, 0)
        else:
            
            return(fit, func)
        
    def biexponential_fit(self,
                          min_val=min_val_default,
                          max_val=max_val_default,
                          initial_conditions=[1, 1e-3, 1, 1e-4],
                          error_func="square difference",
                          **args):
        fit_times = list()
        fit_counts = list()
            
        fit_data = self.fit_data(min_val, max_val)

        for fit_time, fit_count in zip(fit_data.time_bins(), fit_data.counts):
            if fit_count != 0:
                fit_times.append(fit_time)
                fit_counts.append(fit_count)

        fit_times = numpy.array(fit_times)
        fit_counts = numpy.array(fit_counts)

        def biexponential(a1, k1, a2, k2):
            return(lambda x: a1*numpy.exp(-k1*numpy.array(x))\
                   +a2*numpy.exp(-k2*numpy.array(x)))

        def error(params):
            if any(map(lambda x: x < 0, params)):
                return(float("inf"))

            if not sorted(params[1::2]) == list(params[1::2]):
                return(float("inf"))
            
            data = fit_counts
            model = biexponential(*params)(fit_times)

            if error_func == "square difference":
                return(sum(map(lambda x, y: (x-y)**2, data, model)))
            elif error_func == "percent":
                return(sum(map(lambda x, y: abs((x-y)/x), data, model)))
            else:
                raise(ValueError("Unknown error type: {}".format(error_func)))

        fit = scipy.optimize.fmin(error, initial_conditions, **args)

        return(fit, biexponential(*fit))        

    def lifetime(self, min_val=min_val_default, max_val=max_val_default,
                 error=False):
        """
        Perform an exponential fit and report the time constant.
        """
        if not any(self.counts):
            if error:
                return(0, 0)
            else:
                return(0)
            
        if error:
            fit, func, error = self.exponential_fit(min_val=min_val,
                                                    max_val=max_val,
                                                    error=True)
            return(-1/fit[0], error)
        else:
            fit, func = self.exponential_fit(min_val=min_val,
                                             max_val=max_val)
            return(-1/fit[0])

    def mean_arrival_time(self):
        """
        Return the average arrival time for the data, minus the time origin.
        """
        if sum(self.counts) == 0:
            return(0)
        else:
            weighted_sum = sum(map(lambda c, t: c*mean(t), \
                                   self.counts, self.times))            
            pure_sum = float(sum(self.counts))
            
            return(weighted_sum/pure_sum)

    def neighbor_normalized(self):
        times, counts = neighbor_normalize(self.times, self.counts)

        if len(times) < 2:
            return(self)
        else:
            return(Lifetime(counts, times=times))

    def subtract_background(self, threshold=0.1):
        """
        Use the points before the initial rise in the lifetime to measure
        the flat background, and remove it from the data.
        """
        max_counts = max(self.counts)
        max_at = list(self.counts).index(max_counts)

        if max_at == 0:
            return(self)
        
        for index in reversed(range(max_at)):
            if self.counts[index] < threshold*max_counts:
                break

        background = mean(self.counts[:index])

        def apply_background(count):
            if count - background < 0:
                return(0)
            else:
                return(count-background)
            
        return(Lifetime(list(map(apply_background, self.counts)),
                        times=self.times))

    def first_second_emission(self):
        """
        The lifetime represents counts collected from all types of photon
        emission events. In number-resolved methods we can distinguish events
        producing one, two, or more photons, which may contain distinct physics.
        This method produces the lifetimes of the first and second emission
        events, under the assumption that the lifetime represents emission
        from an ensemble of emitters.
        """
        before = 0
        after = sum(self.counts)

        first = list()
        second = list()

        for point in self.counts:
            first.append(point*after)
            second.append(before*point)

            before += point
            after -= point

        return(Lifetime(first, times=self.times),
               Lifetime(second, times=self.times))
 