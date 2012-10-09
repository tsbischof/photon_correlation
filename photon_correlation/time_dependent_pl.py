#!/usr/bin/env python3

import optparse
import csv
import subprocess
import logging

import photon_correlation

def time_dependent_pl(filename, channels, bin_width, time_limits):
    photons = photon_correlation.Picoquant(filename, channels=channels)
    if photons.mode != photon_correlation.modes.T3:
        logging.error("File {0} is not t3 mode.".format(filename))
        return(False)

    photon_streams = photon_correlation.photon.WindowedStream(
        photons, pulse=bin_width)

    for photon_stream in photon_streams:
        dst_filename = "{0}.g1.{1}_{2}".format(filename,
                                               stream.pulse_limits.lower,
                                               stream.pulse_limits.upper)

        photon_correlation.lifetime.lifetime(
            photon_stream,
            time_limits=time_limits,
            filename=dst_filename)
        
    return(True)

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
        time_limits = photon_correlation.histogram.Limits(options.time_limits)
    else:
        raise(ValueError("Must specify time limits."))
    
    for filename in args:
        time_dependent_pl(filename, channels, bin_width, time_limits)
