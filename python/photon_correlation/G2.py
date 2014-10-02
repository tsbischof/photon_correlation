import csv
import fractions

import matplotlib.pyplot as plt

from util import *

t3_center = (-0.5, 0.5)
t3_side = (0.5, 1.5)

class G2(object):
    def __init__(self, filename=None, run_dir=None, stream=None):
        self._counts = dict()

        self._center_side_ratios = None
        self._center_side_ratio = None
        self._autocorrelation = None

        if run_dir is not None:
            self.run_dir = run_dir
            self.filename = "g2"
            
            self.from_file(os.path.join(self.run_dir, self.filename))
        elif filename is not None:
            self.run_dir, self.filename = os.path.split(filename)

            self.from_file(os.path.join(self.run_dir, self.filename))
        elif stream is not None:
            self.run_dir = run_dir
            self.filename = filename

            self.from_stream(stream)
        else:
            pass

    def __getitem__(self, correlation):
        return(self._counts[correlation])

    def __setitem__(self, correlation, gn):
        self._counts[correlation] = gn

    def __iter__(self):
        for correlation in sorted(self._counts.keys()):
            yield(correlation, self._counts[correlation])

    def counts(self):
        if not self._counts:
            self.from_file()

        return(self._counts)

class G2_T3(G2):
    def from_file(self, filename):
        with open(filename) as stream_in:
            return(self.from_stream(csv.reader(stream_in)))

    def from_stream(self, stream_in):
        self._counts = dict()

        for c0, c1, pulse_left, pulse_right, \
            time_left, time_right, counts in stream_in:
            correlation = (int(c0), int(c1))

            pulse_bin = (float(pulse_left), float(pulse_right))
            time_bin = (float(time_left), float(time_right))

            if int(counts) == float(counts):
                counts = int(counts)
            else:
                counts = float(counts)

            if not correlation in self._counts.keys():
                self._counts[correlation] = dict()
                
            if not pulse_bin in self._counts[correlation].keys():
                self._counts[correlation][pulse_bin] = dict()

            self._counts[correlation][pulse_bin][time_bin] = counts

        return(self)

    def pulse_bin_counts(self, correlation, pulse_bin):
        return(sum(self.counts()[correlation][pulse_bin].values()))
    
    def center_side_ratios(self, center=t3_center, side=t3_side):
        if self._center_side_ratios is None:
            self._center_side_ratios = dict()
                
            for correlation, g2 in self:
                center_counts = self.pulse_bin_counts(correlation, center)
                side_counts = self.pulse_bin_counts(correlation, side)

                self._center_side_ratios[correlation] = \
                                        (center_counts, side_counts)
                
        return(self._center_side_ratios)

    def center_side_ratio(self, center=t3_center, side=t3_side):
        """
        Return the center/side ratio formed by summing over
        all cross-correlations in the g2.
        """
        if self._center_side_ratio is None:
            center_total = 0
            side_total = 0
            
            for correlation, ratio in self.g2_ratios().items():               
                if is_cross_correlation(correlation):
                    center, side = ratio
                
                    center_total += center
                    side_total += side

            self._center_side_ratio = float(center_total)/side_total

        return(self._center_side_ratio)

    def autocorrelation(self):
        """
        Add together all of the counts from the cross-correlations to get the
        resulting approximate autocorrelation.
        """        
        if self._autocorrelation is None:
            self._autocorrelation = dict()
            
            for correlation, g2 in self:
                if not is_cross_correlation(correlation):
                    continue

                for pulse_bin in g2.keys():
                    if not pulse_bin in self._autocorrelation.keys():
                        self._autocorrelation[pulse_bin] = dict()

                    for time_bin, counts in g2[pulse_bin].items():
                        if time_bin in self._autocorrelation[pulse_bin].keys():
                            self._autocorrelation[pulse_bin][time_bin] += counts
                        else:
                            self._autocorrelation[pulse_bin][time_bin] = counts

        return(self._autocorrelation)            

    def make_figure(self):
        g2 = self.autocorrelation()

        fig = plt.figure()
            
        ax_center = fig.add_subplot(121)
        ax_side = fig.add_subplot(122)

        max_counts = 0

        for title, pulse_bin, ax in [("center", t3_center, ax_center),
                                     ("side", t3_side, ax_side)]:
            my_g2 = g2[pulse_bin]

            times = list(map(lambda x: mean(x[0])*1e-3,
                             sorted(my_g2.items())))
            counts = list(map(lambda x: x[1],
                              sorted(my_g2.items())))

##            n = 16
##            times = smooth(times, n=n)
##            counts = rebin(counts, n=n)

            my_max = max(counts)
            if my_max > max_counts:
                max_counts = my_max

            max_time = abs(max(times, key=abs))
            
            ax.plot(times, counts)
            ax.set_title("{}: {} counts".format(title, sum(counts)))
            ax.set_xlabel("Time/ns")
            ax.set_xlim((-max_time, max_time))

        for ax in fig.axes:
            ax.set_ylim((0, my_max))

        ax_center.set_ylabel("Counts")
        fig.tight_layout()
        
        return(fig)

    def total_counts(self, pulse_bin):
        """
        Return all counts associated with all cross-correlations and the given
        pulse bin.
        """
        result = None
        
        for correlation, g2 in self:
            if not is_cross_correlation(correlation):
                continue

            if result is None:
                result = g2[pulse_bin]
            else:
                for time_bin, counts in g2[pulse_bin].items():
                    result[time_bin] += counts

        return(result)

class G2_T2(G2):       
    def from_file(self, filename, int_counts=True):
        with open(filename) as stream_in:
            return(self.from_stream(csv.reader(stream_in),
                                    int_counts=int_counts))

    def from_stream(self, stream_in, int_counts=True):
        for c0, c1, time_left, time_right, counts in stream_in:
            correlation = (int(c0), int(c1))
            time_bin = (float(time_left), float(time_right))

            if int_counts:
                counts = int(counts)
            else:
                counts = float(counts)

            if not correlation in self._counts.keys():
                self._counts[correlation] = dict()

            self._counts[correlation][time_bin] = counts

        return(self)

    def autocorrelation(self):
        if self._autocorrelation is None:
            result = dict()
            
            for correlation, g2 in self:
                if not is_cross_correlation(correlation):
                    continue

                for time_bin, counts in g2.items():
                    if time_bin not in result.keys():
                        result[time_bin] = 0

                    result[time_bin] += counts

            self._autocorrelation = result
            
        return(self._autocorrelation)

    def to_resolution(self, resolution):
        result = G2_T2()

        for correlation, g2 in self:
            times = list(map(lambda x: x[0], sorted(g2.items())))
            counts = list(map(lambda x: x[1], sorted(g2.items())))

            current_resolution = int(times[0][1] - times[0][0])
            binning = resolution / current_resolution

            if binning != 1:
                times = list(zip(smooth(map(lambda x: x[0], times), n=binning),
                                 smooth(map(lambda x: x[1], times), n=binning)))
                             
                counts = rebin(counts, n=binning)

                my_g2 = dict()
                
                for time_bin, count in zip(times, counts):
                    my_g2[time_bin] = count
            else:
                my_g2 = g2
                
            result[correlation] = my_g2

        return(result)

    def make_figure(self):
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)

        g2 = sorted(self.autocorrelation().items())

        times = list(map(lambda x: mean(x[0])*1e-3, g2))
        counts = list(map(lambda x: x[1], g2))

        ax.plot(times, counts)
        ax.set_ylabel("$g^{(2)}$")
        ax.set_xlabel("Time/ns")
        ax.set_xlim((times[0], times[-1]))

        fig.tight_layout()
    
        return(fig)
    
