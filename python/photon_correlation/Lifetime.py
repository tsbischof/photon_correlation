import bisect
import math
import statistics

import numpy
import scipy.optimize

from .util import *
from .Exponential import *

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

    def __sub__(self, other):
        return(self + Lifetime(list(map(lambda x: -x, other.counts)),
                               times=other.times))

    def __iter__(self):
        return(iter(zip(self.times, self.counts)))

    @property
    def time_bins(self):
        return(list(map(statistics.mean, self.times)))

    def normalized(self, key=max):
        """
        Return the normalized lifetime, based on the given function.
        """
        return(Lifetime(list(normalize(self.counts, key=key)),
                        times=self.times))

    def to_resolution(self, resolution=1024):
        """
        Perform the rebinning necessary to achieve the desired resolution,
        and return a new lifetime object.
        """
        if not resolution:
            return(self)
        
        binning = int(resolution / self.resolution)

        if binning < 2:
            return(self)
        else:
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
        index_lower = bisect.bisect_left(self.time_bins, lower)
        index_upper = bisect.bisect_left(self.time_bins, upper)

        return(Lifetime(self.counts[index_lower:index_upper],
                        times=self.times[index_lower:index_upper]))

    def origin(self):
        """
        Return the index at which the curve is maximal.
        """
        return(list(self.counts).index(max(self.counts)))

    def fit_data(self, min_val=min_val_default, max_val=max_val_default):
        """
        Find the range associated with the given threshold of values to
        perform a fit. Use the data which vall after the counts have decayed
        to max_val of their maximum value, but before they reach min_val of
        their maximum value.
        """
        if min_val is None:
            min_val = 0

        if max_val is None:
            max_val = 1
            
        # only look after the maximum
        origin = self.origin()
        
        max_counts = max(self.counts)

        counts_lower = min(self.counts[origin:],
                           key=lambda x: abs(x - max_counts*min_val))
        counts_upper = min(self.counts[origin:],
                           key=lambda x: abs(x - max_counts*max_val))

        index_left = last_index(self.counts, counts_upper)
        index_right = first_index(self.counts, counts_lower)

        return(self.range(self.time_bins[index_left],
                          self.time_bins[index_right]))
        
    def exponential_fit(self,
                        min_val=None,
                        max_val=None,
                        time_range=None,
                        n_exponentials=1,
                        initial_conditions=None,
                        error_func="square difference",
                        **args):
        if initial_conditions is None:
            initial_conditions = [1, 1]*n_exponentials
        else:
            n_exponentials = len(initial_conditions) / 2

        if n_exponentials*2 != len(initial_conditions):
            raise(ValueError("Dimension mismatch: expected {} parameters for "
                             "{} exponentials, but got {}".format(
                                 n_exponentials*2, n_exponentials,
                                 len(initial_conditions))))
        
        fit_times = list()
        fit_counts = list()

        if min_val is not None or max_val is not None:
            fit_data = self.fit_data(min_val, max_val)
        elif time_range is not None:
            fit_data = self.range(*time_range)
        else:
            fit_data = self

        for fit_time, fit_count in zip(fit_data.time_bins, fit_data.counts):
            if fit_count != 0:
                fit_times.append(fit_time)
                fit_counts.append(fit_count)

        fit_times = numpy.array(fit_times)
        fit_counts = numpy.array(fit_counts)

        def error(params):
            if any(map(lambda x: x < 0, params)):
                return(float("inf"))

            if not sorted(params[1::2]) == list(params[1::2]):
                return(float("inf"))
            
            data = fit_counts
            model = MultiExponential(params)(fit_times)

            if error_func == "square difference":
                return(sum(map(lambda x, y: (x-y)**2, data, model)))
            elif error_func == "percent":
                return(sum(map(lambda x, y: abs((x-y)/x) if x > 0 else 0,
                               data, model)))
            else:
                raise(ValueError("Unknown error type: {}".format(error_func)))
        
        fit = scipy.optimize.fmin(error, initial_conditions, **args)

        return(MultiExponential(fit))

    def fit(self, fit_f, p0=None, error="least squares", params_check=None,
            force_calculation=False, **fmin_args):
        if error == "least squares":
            err_f = lambda x, y: (x-y)**2
        elif error == "percent":
            err_f = lambda x, y: abs((x-y)/x) if x > 0 else 0
        else:
            raise(ValueError("Unknown error: {}".format(error)))
                
        t = self.time_bins
        data = self.counts

        def error(p):
            if params_check is not None and not params_check(p):
                return(float("inf"))
            else:
                return(sum(map(err_f, fit_f(p), data)))
            
        if force_calculation:
            params = scipy.optimize.fmin(error, p0,
                                         **fmin_args)
        else:
            params = p0

        return(params, fit_f(params))

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
            func = self.exponential_fit(min_val=min_val,
                                             max_val=max_val)
            return(-1/func[0].rate)

    def mean_arrival_time(self):
        """
        Return the average arrival time for the data, minus the time origin.
        """
        if sum(self.counts) == 0:
            return(0)
        else:
            weighted_sum = sum(map(lambda c, t: c*statistics.mean(t), \
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

        background = statistics.mean(self.counts[:index])

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
        return(util.pnrl_2_from_signals(self.counts, self.counts,
                                        times=self.times))
    
##        before = 0
##        after = sum(self.counts)
##
##        first = list()
##        second = list()
##
##        for point in self.counts:
##            first.append(point*after)
##            second.append(before*point)
##
##            before += point
##            after -= point
##
##        return(Lifetime(first, times=self.times),
##               Lifetime(second, times=self.times))
## 
