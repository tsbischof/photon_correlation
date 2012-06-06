#!/usr/bin/env python3

import itertools
import csv
import fractions
import math
import functools
import collections
import optparse
import logging
import os
import subprocess
import copy

PICOQUANT = "picoquant"
INTENSITY = "intensity"
CORRELATE = "correlate"
HISTOGRAM = "histogram"

T2 = "t2"
T3 = "t3"

class Limits(object):
    def __init__(self, limit_str):
        limits = limit_str.split(",")
        # Handle conversion from ms to ps.
        self.lower = int(float(limits[0])*10**9)
        self.bins = int(limits[1])
        self.upper = int(float(limits[2])*10**9) 

    def __str__(self):
        return(",".join(map(str, [self.lower, self.bins, self.upper])))

class HistogramBin(object):
    def __init__(self, mode=None, ref_channel=None, bins=tuple(), counts=None,
                 time_resolution=1, pulse_resolution=1):
        self.mode = mode
        self.ref_channel = ref_channel
        self._bins = tuple(bins)
        self.counts = counts

        self.time_resolution = time_resolution
        self.pulse_resolution = pulse_resolution

    def from_line(self, mode, channels, order, line):
        self.line = line
        self.mode = mode
        self.ref_channel = int(self.line[0])
        self.counts = int(self.line[-1])
        self._bins = tuple(get_bins(self.line[1:-1], mode))
        return(self)

    def to_line(self):
        result = list()

        channels = self.channels()
        bins = self.time_bins()
        
        result.append(str(self.ref_channel))

        for i in range(len(self.channels())):
            result.append(str(channels[i]))
            for dimension in bins[i]:
                result.append(str(dimension.bounds[0]))
                result.append(str(dimension.bounds[1]))

        result.append(str(self.counts))

        return(result)

    def time_bins(self):
        return(tuple(map(lambda x: x[1:], self._bins)))

    def channels(self):
        return(tuple(map(lambda x: x[0], self._bins)))

    def correlation(self):
        return(tuple([self.ref_channel] + list(self.channels())))

    def __hash__(self):
        return(hash(self.line))

    def volume(self):
        volume = 1
        for my_bin in self.time_bins():
            for bounds in my_bin:
                volume *= len(bounds)

        return(volume)
          
class TimeBin(object):
     def __init__(self, bounds, resolution=1):
          self.bounds = bounds
          self.resolution = resolution

     def __str__(self):
          return(str(self.bounds))

     def __len__(self):
          return(int(math.ceil(float(self.bounds[1])/self.resolution)
                     - math.floor(float(self.bounds[0])/self.resolution)))
          
     def __lt__(self, other):
          return(self.bounds < other.bounds)

     def __gt__(self, other):
          return(self.bounds > other.bounds)

     def __eq__(self, other):
          return(self.bounds == other.bounds)

     def __le__(self, other):
          return(self.bounds <= other.bounds)

     def __ge__(self, other):
          return(self.bounds >= other.bounds)

     def __hash__(self):
          return(self.bounds.__hash__())
               
class CrossCorrelations(object):
    def __init__(self, filename, mode, channels, order,
                 time_resolution=1, pulse_resolution=1):
        self._filename = filename
        self.mode = mode
        self.channels = channels
        self.order = order
        self._bins = list()
        self._bins_cross_norm = list()
        self._bins_auto = list()
        self._bins_auto_norm = list()

        self.time_resolution = time_resolution
        self.pulse_resolution = pulse_resolution

    def add_bin(self, my_bin):
        self._bins.append(my_bin)

    def bins(self):
        if not self._bins:
            logging.debug("Loading bins from {0}".format(self._filename))
            with open(self._filename) as stream:
                self.from_stream(stream)

        return(self._bins)

    def cross_correlations(self, intensities=None, normalize=False):
        if normalize:
            dst_filename = "{0}.norm".format(self._filename)

            correlation_intensity = dict()

            with open(dst_filename, "w") as out_stream:
                writer = csv.writer(out_stream)

                print(intensities)
                integration_time = max(map(lambda x: x[1], intensities.values()))
                
                for src_bin in self.bins():
                    my_bin = copy.deepcopy(src_bin)
                    correlation = my_bin.correlation()

                    # We need to divide by the average intensity and the
                    # volume of phase space represented by the bin.
                    if not correlation in correlation_intensity.keys():
                        correlation_intensity[correlation] = 1

                        for channel in correlation:
                            correlation_intensity[correlation] *= \
                                      fractions.Fraction(*intensities[channel])

                    volume = my_bin.volume()
                    intensity = correlation_intensity[correlation]

                    try:
                        my_bin.counts = float(
                            fractions.Fraction(my_bin.counts) \
                                 / (volume * intensity * integration_time))
                    except ZeroDivisionError:
                        my_bin.counts = 0

                    writer.writerow(my_bin.to_line())


                
                    

                
            
##            intensity_normalization = dict()
##            for correlation in all_cross_correlations(channels, order):
##                intensity_normalization[correlation] = fractions.Fraction(1,1)
##
##                for channel in correlation:
##                    intensity_normalization[correlation] *= \
##                                fractions.Fraction(intensities[channel][0],
##                                                   intensities[channel][1])
##
##            for cross_bin in self.bins:
##                try:
##                    cross_bin.counts /= intensity_normalization[\
##                        cross_bin.correlation]
##                    cross_bin.space_normalize()
##                except ZeroDivisionError:
##                    cross_bin.counts = 0
##
##                self._bins_cross_norm.append(cross_bin)                
##                
            logging.info("Normalized cross-correlations written to {0}".format(
                dst_filename))
        else:
            logging.info("Cross-correlations written to {0}".format(
                self._filename))

    def autocorrelation(self, intensities=None, normalize=False):
        if normalize:
            dst_filename = "{0}.auto.norm".format(self._filename)
            logging.info("Normalized autocorrelation written to {0}".format(
                dst_filename))

        else:
            dst_filename = "{0}.auto".format(self._filename)

            logging.info("Autocorrelation written to {0}".format(
                dst_filename))


    def from_stream(self, stream):
        for line in csv.reader(stream):
            self.add_bin(HistogramBin().from_line(self.mode,
                                                  self.channels,
                                                  self.order,
                                                  line))







def guess_mode(filename):
    if filename.lower().endswith(T2):
        return(T2)
    elif filename.lower().endswith(T3):
        return(T3)
    else:
        raise(ValueError("Mode could not be identified "
                         "for {0}.".format(filename)))

def flatten(LoL):
     for elem in LoL:
          if isinstance(elem, collections.Iterable):
               for minor in flatten(elem):
                    yield(minor)
          else:
               yield(elem)

def chunks(L, n=2):
     chunk = list()
     for elem in L:
          chunk.append(elem)
          if len(chunk) >= n:
               yield(chunk)
               chunk = list()

def all_cross_correlations(channels, order):
     return(
          itertools.product(
               range(channels),
               repeat=order))

def get_bins(bins, mode, time_resolution=1, pulse_resolution=1):
     if mode == T2:
          for chunk in chunks(bins, 3):
               channel = int(chunk[0])
               bounds = (float(chunk[1]), float(chunk[2]))
               yield((channel,
                      TimeBin(bounds,
                              resolution=time_resolution)))
     elif mode == T3:
          for chunk in chunks(bins, 5):
              channel = int(chunk[0])
              pulse_bounds = (float(chunk[1]), float(chunk[2]))
              time_bounds = (float(chunk[3]), float(chunk[4]))
              yield(channel,
                    TimeBin(pulse_bounds,
                            resolution=pulse_resolution),
                    TimeBin(time_bounds,
                            resolution=time_resolution))
     else:
          raise(ValueError("Mode {0} not recognized.".format(mode)))
        
def get_resolution_cmd(filename):
    return([PICOQUANT, "--file-in", filename, "--resolution-only"])

def get_resolution(filename):
    raw_resolution = subprocess.Popen(get_resolution_cmd(filename),
                                      stdout=subprocess.PIPE).stdout.read()
    return(int(math.floor(float(raw_resolution))))

def get_photon_stream_cmd(filename, number, print_every):                         
    # Build the photon stream command.
    photon_stream_cmd = [PICOQUANT,
                         "--file-in", filename]
    if number:
        photon_stream_cmd.extend(["--number", str(number)])
    if print_every:
        photon_stream_cmd.extend(["--print-every", str(print_every)])

    return(photon_stream_cmd)

def get_correlate_cmd(filename, mode, order, time_limits, pulse_limits,
                      time_scale, pulse_scale):
    # Build the correlation command.
    correlate_cmd = [CORRELATE,
                     "--mode", mode,
                     "--order", str(order),
                     "--max-time-distance",
                              str(max([abs(time_limits.lower),
                                       abs(time_limits.upper)])-1)]
    if mode == T3:
        correlate_cmd.extend(["--max-pulse-distance",
                              str(max([abs(pulse_limits.lower),
                                       abs(pulse_limits.upper)])-1)])

    if "log" in time_scale or "log" in pulse_scale:
        correlate_cmd.extend(["--positive-only"])

    return(correlate_cmd)

def get_histogram_cmd(filename, dst_filename, mode, channels, order,
                      time_limits, pulse_limits,
                      time_scale, pulse_scale):
    # Build the histogram command.
    histogram_cmd = [HISTOGRAM,
                     "--file-out", dst_filename,
                     "--mode", mode,
                     "--order", str(order),
                     "--channels", str(channels),
                     "--time", str(time_limits),
                     "--time-scale", time_scale]
    if mode == T3:
        histogram_cmd.extend(["--pulse", str(pulse_limits),
                              "--pulse-scale", pulse_scale])

    return(histogram_cmd)

def get_intensity_cmd(filename, dst_filename, mode, channels):
    return([INTENSITY,
            "--mode", mode,
            "--channels", str(channels),
            "--count-all",
            "--file-out", dst_filename])

def get_intensities(filename, mode, channels, number, print_every):
    dst_filename = "{0}.sum_intensity".format(filename)
    if not os.path.isfile(dst_filename):
        logging.info("Counting the number of photons on each channel.")
        photon_stream = subprocess.Popen(get_photon_stream_cmd(filename,
                                                               number,
                                                               print_every),
                                         stdout=subprocess.PIPE)
        subprocess.Popen(get_intensity_cmd(filename,
                                           dst_filename,
                                           mode,
                                           channels),
                         stdin=photon_stream.stdout).wait()

    with open(dst_filename) as stream:
        line = next(csv.reader(stream))
        time_delta = int(line[1]) - int(line[0])
        counts = map(int, line[2:])

        intensities = dict()
        for channel, count in enumerate(counts):
            intensities[channel] = (count, time_delta)

    return(intensities)

def get_histograms(filename, mode, channels, order,
                   time_limits, pulse_limits,
                   time_scale, pulse_scale,
                   number, print_every):
    dst_filename = "{0}.g{1}".format(filename, order)
    if not os.path.isfile(dst_filename):
        logging.info("Performing the cross-correlation. This may take a while.")
        photon_stream = subprocess.Popen(
            get_photon_stream_cmd(filename,
                                  number,
                                  print_every),
            stdout=subprocess.PIPE)
        correlate_stream = subprocess.Popen(
            get_correlate_cmd(filename,
                              mode, order,
                              time_limits, pulse_limits,
                              time_scale, pulse_scale),
            stdin=photon_stream.stdout, stdout=subprocess.PIPE)
        histogram_stream = subprocess.Popen(
            get_histogram_cmd(filename, dst_filename,
                              mode, channels, order,
                              time_limits, pulse_limits,
                              time_scale, pulse_scale),
            stdin=correlate_stream.stdout).wait()

    # histograms now exist, load them into memory for great profit.
    return(CrossCorrelations(dst_filename, mode, channels, order))
                              
def calculate_gn(filename, mode, channels, order,
                     time_limits, pulse_limits,
                     time_scale, pulse_scale,
                     number, print_every, normalize):
    logging.info("Processing {0}".format(filename))
    if not mode:
        try:
            mode = guess_mode(filename)
        except Exception as error:
            logging.error(error)
            return(False)

    if mode == T3 and not pulse_limits:
        logging.error("Must specify pulse limits for t3 data.")
        return(False)

    # Gather the histograms.

    histograms = get_histograms(filename, mode, channels, order,
                                time_limits, pulse_limits,
                                time_scale, pulse_scale,
                                number, print_every)
    histograms.autocorrelation()

    if normalize:
        normalize_histograms(histograms, filename,
                             mode, channels, order,
                             number, print_every)

def normalize_histograms(histograms, filename, order,
                         mode, channels, number, print_every):
    time_resolution = get_resolution(filename)
    pulse_resolution = 1
        
    intensities = get_intensities(filename, mode, channels,
                                  number, print_every)

    histograms.autocorrelation(intensities=intensities,
                               normalize=True)
    histograms.cross_correlations(intensities=intensities,
                                  normalize=True)
    
if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)

    usage = "gn.py [options] filenames"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-m", "--mode", dest="mode",
                      help="Mode the data is formatted as, either t2 or t3.",
                      action="store")
    parser.add_option("-n", "--number", dest="number",
                       help="Number of entries to process. By default, all "
                           "entries are processed.",
                       action="store", type=int)
    parser.add_option("-c", "--channels", dest="channels",
                      help="Number of channels in the data. The default is 2.",
                      action="store", type=int, default=2)
    parser.add_option("-g", "--order", dest="order",
                      help="Order of the correlation to run. The default is 2.",
                      action="store", type=int, default=2)
    parser.add_option("-d", "--time", dest="time_limits",
                      help="Specify the time limits for a histogram run, "
                          "as lower,bins,upper.",
                      action="store")
    parser.add_option("-e", "--pulse", dest="pulse_limits",
                      help="Specify the pulse limits for a histogram run.",
                      action="store")
    parser.add_option("-D", "--time-scale", dest="time_scale",
                      help="Scale for the time axis of a histogram run. Can "
                      "be linear, log, or log-zero (includes zero-time bin)",
                      default="linear", action="store")
    parser.add_option("-E", "--pulse-scale", dest="pulse_scale",
                      help="Scale for the pulse axis, same choices as "
                           "time-scale.",
                      default="linear", action="store")
    parser.add_option("-p", "--print-every", dest="print_every",
                      help="Print the record number whenever it is divisible "
                      "by this number. By default, nothing is printed.",
                      default=0, action="store", type=int)
    parser.add_option("-N", "--no-normalize", action="store_true",
                      help="Suppress the usual normalization routine.",
                      default=False)

    options, args = parser.parse_args()
    logging.debug("Options: {0}".format(options))
    logging.debug("Arguments: {0}".format(args))

    if options.mode:
        mode = options.mode.lower()
    else:
        mode = options.mode    

    number = options.number
    channels = options.channels
    order = options.order
    
    if options.time_limits:
        time_limits = Limits(options.time_limits)
    else:
        raise(ValueError("Must specify time limits."))

    try:
        pulse_limits = Limits(options.pulse_limits)
    except:
        pulse_limits = options.pulse_limits
        
    time_scale = options.time_scale
    pulse_scale = options.pulse_scale
    print_every = options.print_every
    normalize = not options.no_normalize

    for filename in args:
        calculate_gn(filename, mode, channels, order,
                     time_limits, pulse_limits,
                     time_scale, pulse_scale,
                     number, print_every, normalize)

    
