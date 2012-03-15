#!/usr/bin/env python

import subprocess
import sys
import re
import optparse
import os
import logging
import math

picoquant = "picoquant"
intensity = "intensity"
histogram = "histogram"
correlate = "correlate"

picoquant_modes = {
    "thd": "interactive",
    "thc": "interactive",
    "t3r": "t3",

    "phd": "interactive",
    "pt2": "t2",
    "pt3": "t3",

    "hhd": "interactive",
    "ht2": "t2",
    "ht3": "t3"}

picoquant_channels = {
    "t": 2,
    "p": 2,
    "h": 4}

tttr_modes = ["t2", "t3"]

class Picoquant:
    def __init__(self, filename, options):
        self.filename = filename
        self.src_dir, self.src_filename = os.path.split(filename)
        self.filename_base = self.filename[:-4]
        self.file_type = self.filename[-3:]
        self.options = options
        logging.info("Processing {0}.".format(filename))
        logging.debug("Creating picoquant object from {0}".format(filename))

        self.data_cmd = [picoquant, "--file-in", filename]
        if self.options.number > 0:
            logging.debug("Processing {0} entries.".format(self.options.number))
            self.data_cmd.extend(["--number", str(self.options.number)])

        if self.mode() == None:
            raise(ValueError("Mode could not be parsed: {0}".format(
                self.options.mode)))
        else:
            logging.debug("Found mode {0}".format(self.mode()))

    def resolution(self):
        resolution = subprocess.Popen(self.data_cmd + ["--resolution"],
                                      stdout=subprocess.PIPE).stdout.read()
        try:
            return(float(resolution))
        except:
            raise(ValueError("Resolution could not be converted"
                             "to float: {0}".format(resolution)))

    def mode(self):
        if self.options.mode:
            mode = self.options.mode
            guess = guess_mode(self.filename)
            if mode != guess:
                if not mode in picoquant_modes.values():
                    raise(ValueError("Specified mode does not appear to be "
                                     "a tttr mode: {0}".format(mode)))
                logging.warn("Input mode {0} does not match guessed "
                             "mode {1}.".format(mode, guess))
        else:
            mode = guess_mode(self.filename)

        return(mode)

    def is_tttr(self):
        return(self.mode() in tttr_modes)

    def channels(self):
        if self.options.channels:
            return(self.options.channels)
        else:
            logging.debug("Guessing the number of channels from the file type.")
            return(guess_channels(self.file_type))

    def integer_time(self, float_time):
        "Return the time specified as a float number of milliseconds as "
        "the integer number relevant for the particular resolution of the "
        "file."
        return(int(math.floor(float_time*self.resolution()*1e9)))

    def run_intensity(self):
        logging.info("Running an intensity trace.")

        intensity_dst = "{0}.intensity".format(self.filename)
        logging.debug("Intensity trace will be stored at {0}".format(
            intensity_dst))

        if self.mode() == "t2":
            intensity_bin_width = self.integer_time(
                    self.options.intensity_bin_width)
            logging.debug("Intensity bin width: {0} ({1}ms)".format(
                intensity_bin_width, self.options.intensity_bin_width))
        elif self.mode() == "t3":
            intensity_bin_width = self.options.intensity_bin_width
            logging.debug("Intensity bin width: {0} pulses.".format(
                intensity_bin_width))
        else:
            raise(TypeError("Itensity trace for mode {0} could not "
                            "be handled.".format(self.mode())))

        intensity_cmd = [intensity,
                        "--bin-width", str(intensity_bin_width),
                        "--channels", str(self.channels()),
                        "--file-out", intensity_dst,
                        "--mode", self.mode()]
        
        data = subprocess.Popen(self.data_cmd, stdout=subprocess.PIPE)        
        subprocess.Popen(intensity_cmd, stdin=data.stdout).wait()
    
    def run_correlation(self):
        pass

    def run_interactive(self):
        pass
        

def guess_mode(filename):
    extension = filename[-3:]
    try:
        return(picoquant_modes[extension])
    except:
        return(None)

def guess_channels(file_type):
    board_type = file_type[0]
    try:
        return(picoquant_channels[board_type])
    except KeyError:
        raise(KeyError("Board type '{0}' not recognized for "
                       "file type {1}".format(board_type, file_type)))

def process(filename, options):
    # Typical tasks to run:
    # - intensity
    # - g(2)(t)
    data = Picoquant(filename, options)

    if data.is_tttr():
        data.run_intensity()
        data.run_correlation()
    elif data.is_interactive():
        data.run_interactive()
    else:
        raise(TypeError("Could not identify file type for {0}".format(
            filename)))

if __name__ == "__main__":    
    parser = optparse.OptionParser()
    parser.add_option("-m", "--mode", dest="mode",
                      help="Mode to interpret the data as. By default, "
                          "the program will attempt to guess the mode.",
                      action="store")
    parser.add_option("-w", "--intensity-bin-width", dest="intensity_bin_width",
                      help="Set the bin width for an intensity run, in ms (t2) "
                          "or pulses (t3).",
                      action="store", type=float)
    parser.add_option("-n", "--number", dest="number",
                       help="Number of entries to process. By default, all "
                           "entries are processed.",
                       action="store", type=int)
    parser.add_option("-c", "--channels", dest="channels",
                      help="Number of channels in the data. The default is "
                          "to guess the number from the file type.",
                      action="store", type=int)
    parser.add_option("-g", "--order", dest="order",
                      help="Order of the correlation to run. The default is 2.",
                      action="store", type=int)
    parser.add_option("-v", "--verbose", dest="verbose",
                      help="Print debug-level information.",
                      action="store_true", default=False)
    parser.add_option("-q", "--quiet", dest="quiet",
                      help="Suppress all non-vital messages.",
                      action="store_true", default=False)

    (options, args) = parser.parse_args()

    if options.quiet:
        pass
    elif options.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    
    logging.debug("Found options: {0}".format(options))
    logging.debug("Found files to process: {0}".format(options))
    
    for filename in args:
        process(filename, options)
    
