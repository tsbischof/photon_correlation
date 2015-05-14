#!/usr/bin/env python3

import optparse
import csv
import subprocess
import logging
import operator
import os
import re

from photon_correlation import Picoquant
from photon_correlation.histogram import Limits
from photon_correlation.lifetime import time_dependent_lifetime

def tdpl(filename, time_limits, bin_width):
    run_dir = "{0}.g1.td".format(filename)
    for limits, lifetime in time_dependent_lifetime(
        Picoquant(filename),
        time_limits,
        bin_width,
        run_dir=run_dir,
        filename=filename):
        logging.info("Current upper bound: {0}".format(limits.upper))
    

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    
    usage = "time_dependent_pl.py [options] filenames"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-w", "--bin-width", dest="bin_width",
                      help="Set the bin width for the intensity run, "
                           "in pulses.",
                      action="store", type=float, default=500000)
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
        time_limits = Limits(options.time_limits)
    else:
        raise(ValueError("Must specify time limits."))
    
    for filename in args:
        tdpl(filename, time_limits, bin_width)

        
