#!/usr/bin/env python

import optparse
import csv
import subprocess
import logging
import tempfile
import time
import os

PICOQUANT = "picoquant"
CORRELATE = "correlate"
HISTOGRAM = "histogram"

class Limits(object):
    def __init__(self):
        self.lower = None
        self.bins = None
        self.upper = None

    def __str__(self):
        return("{0},{1},{2}".format(self.lower, self.bins, self.upper))

    def from_string(self, string):
        raw_limits = string.split(",")
        self.lower = int(raw_limits[0])
        self.bins = int(raw_limits[1])
        self.upper = int(raw_limits[2])
        return(self)

def correlate_photons(filename, channels, time_limits, photons):
    photons_filename = "photons.temp"
    with open(photons_filename, "w") as photon_file:
        writer = csv.writer(photon_file)
        for photon in photons:
            writer.writerow(map(str, photon))

    with open(photons_filename, "r") as photon_file:
        correlator = subprocess.Popen(
            [CORRELATE,
             "--mode", "t2",
             "--max-time-distance", str(max([time_limits.lower,
                                             time_limits.upper])
                                        -1)],
            stdin=photon_file,
            stdout=subprocess.PIPE)
        histogrammer = subprocess.Popen(
            [HISTOGRAM,
             "--mode", "t2",
             "--channels", str(channels),
             "--time", str(time_limits),
             "--file-out", filename],
            stdin=correlator.stdout).wait()

    os.remove(photons_filename)

def time_dependent_g2(filename, channels, bin_width, time_limits):
    photon_stream = csv.reader(
        subprocess.Popen([PICOQUANT,
                          "--file-in", filename],
                         stdout=subprocess.PIPE).stdout)

    time_limit = bin_width

    done = False
    photon = None
    photons = None

    while not done:
        if not photons:
            histogram_name = "{0}.g2.{1:.2f}_{2:.2f}".format(
                filename, (time_limit-bin_width)*10**-12,
                time_limit*10**-12)
            logging.info(histogram_name)
            photons = list()
            
        if not photon:
            # Try getting a photon, if we have none.
            try:
                photon = next(photon_stream)
            except StopIteration:
                # No more photons, kill and get things done.
                correlate_photons(histogram_name, channels,
                                  time_limits, photons)
                photons = None
                done = True
        
        if photon:
            channel, arrival_time = photon
            if int(arrival_time) < time_limit:
                photons.append(photon)
                photon = None
            else:
                # Outside this histogram, move on to the next.
                correlate_photons(histogram_name, channels,
                                  time_limits, photons)
                photons = None
                time_limit += bin_width
        

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    
    usage = "time_dependent_pl.py [options] filenames"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-w", "--bin-width", dest="bin_width",
                      help="Set the bin width for the intensity run, "
                           "in s.",
                      action="store", type=float, default=1)
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
    bin_width = int(options.bin_width*10**12)

    if options.time_limits:
        time_limits = Limits().from_string(options.time_limits)
    else:
        raise(ValueError("Must specify time limits."))
    
    for filename in args:
        time_dependent_g2(filename, channels, bin_width, time_limits)
