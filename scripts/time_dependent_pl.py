#!/usr/bin/env python

import optparse
import csv
import subprocess
import logging

picoquant = "picoquant"
histogram = "histogram"

def time_dependent_pl(filename, channels, bin_width, time_limits):
    photon_stream = csv.reader(
        subprocess.Popen([picoquant,
                          "--file-in", filename],
                         stdout=subprocess.PIPE).stdout)
    histogram_cmd = [histogram,
                     "--time", time_limits,
                     "--channels", str(channels),
                     "--mode", "t3",
                     "--order", "1"]

    pulse_limit = bin_width
    histogram_index = 0

    histogrammer = None
    done = False
    photon = None

    while not done:
        if not histogrammer:
            histogram_name = "{0}.g1.{1:.2f}_{2:.2f}".format(
                filename, histogram_index*bin_width,
                (histogram_index+1)*bin_width)
            logging.info(histogram_name)
            current_histogram = subprocess.Popen(
                histogram_cmd +
                ["--file-out", histogram_name],
                 stdin=subprocess.PIPE)
            histogrammer = csv.writer(current_histogram.stdin)

        if not photon:
            # Try getting a photon, if we have none.
            try:
                photon = next(photon_stream)
            except StopIteration:
                # No more photons, kill and get things done.
                current_histogram.stdin.close()
                histogrammer = None
                histogram_index += 1
                done = True
        
        if photon:
            channel, arrival_pulse, arrival_time = photon
            if int(arrival_pulse) < pulse_limit:
                histogrammer.writerow(photon)
                photon = None
            else:
                # Outside this histogram, move on to the next.
                current_histogram.stdin.close()
                histogrammer = None
                histogram_index += 1
                pulse_limit += bin_width
        

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    
    usage = "time_dependent_pl.py [options] filenames"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-w", "--bin-width", dest="bin_width",
                      help="Set the bin width for the intensity run, "
                           "in pulses.",
                      action="store", type=float, default=50)
    parser.add_option("-c", "--channels", dest="channels",
                      help="Number of channels in the data. The default is 2.",
                      action="store", type=int, default=2)
    parser.add_option("-H", "--threshold", dest="threshold",
                      help="Threshold rate of photon arrival for blinking "
                           "analysis, in counts per second.",
                      action="store", type=float)
    parser.add_option("-d", "--time", dest="time_limits",
                      help="Time bounds for the histograms, as required by "
                           "histogram.",
                      action="store")

    (options, args) = parser.parse_args()

    channels = options.channels
    bin_width = options.bin_width

    if options.time_limits:
        time_limits = options.time_limits
    else:
        raise(ValueError("Must specify time limits."))
    
    for filename in args:
        time_dependent_pl(filename, channels, bin_width, time_limits)
