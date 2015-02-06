import matplotlib.pyplot as plt
import numpy

from .util import *

class Blinking(object):
    def __init__(self, intensity):
        self.intensity = intensity

        self._on_off_times = dict()

    def dt(self):
        return(self.intensity.dt())

    def on_off_times(self, threshold):
        if threshold not in self._on_off_times.keys():
            times = self.intensity.times
            counts = self.intensity[0]

            on_times = list()
            off_times = list()

            thresheld = list(map(lambda x: x > threshold, counts))
            
            last_switch = None
            last_status = thresheld[0]

            for time_bin, status in zip(times,
                                        map(lambda x: x > threshold, counts)):
                if status != last_status:
                    # Switched
                    if last_switch is None:
                        # Discard the first and last ones
                        pass
                    else:
                        dt = time_bin[0] - last_switch[0]
                       
                        if status:
                            on_times.append(dt)
                        else:
                            off_times.append(dt)

                    last_switch = time_bin
                    last_status = status

                self._on_off_times[threshold] = (on_times, off_times)

        return(self._on_off_times[threshold])

    def make_figure(self, threshold, n_bins=50):
        on_times, off_times = self.on_off_times(threshold)

        fig = plt.figure()
##        ax_on = fig.add_subplot(2, 1, 1)
##        ax_off = fig.add_subplot(2, 1, 2)
##
##        max_time = max((max(on_times), max(off_times)))
##        bins = numpy.logspace(numpy.log10(self.dt()), numpy.log10(max_time),
##                              n_bins)
##
##        for name, ax, times in [("on", ax_on, on_times),
##                                ("off", ax_off, off_times)]:
##            my_counts, my_bins = numpy.histogram(times, bins=bins)
##            my_bins, my_counts = neighbor_normalize(my_bins[:-1], my_counts)
##          
##            ax.plot(my_bins, normalize(my_counts))
##            
##            ax.set_yscale("log")
##            ax.set_xscale("log")
##            ax.set_xlabel("Time/{}".format(self.intensity.time_unit()))
##            ax.set_ylabel("Relative probability density")
##            ax.set_title("{} {} events (threshold: {} counts/{}".format(
##                len(times), name, threshold, self.intensity.time_unit()))
##            ax.set_xlim(bins[0], bins[-1])

        ax = fig.add_subplot(1, 1, 1)

        if on_times and off_times:
            max_time = max((max(on_times), max(off_times)))
            bins = numpy.logspace(numpy.log10(self.dt()), numpy.log10(max_time),
                                  n_bins)

            for name, times in [("on", on_times),
                                ("off", off_times)]:
                my_counts, my_bins = numpy.histogram(times, bins=bins)            
                my_bins, my_counts = neighbor_normalize(my_bins[:-1], my_counts)

                if not any(my_counts):
                    continue
              
                ax.plot(my_bins, normalize(my_counts), label=name)

            ax.legend()
            ax.set_xlim(bins[0], bins[-1])
            
        ax.set_yscale("log")
        ax.set_xscale("log")
        ax.set_xlabel("Time/{}".format(self.intensity.time_unit()))
        ax.set_ylabel("Relative probability density")
        ax.set_title("Threshold {}: {} on, {} off".format(
            threshold, len(on_times), len(off_times)))
    
        fig.tight_layout()

        return(fig)


    
