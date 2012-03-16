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

class Limits:
    def __init__(self, limits_str):
        self.limits = limits_str.split(",")

    def __str__(self):
        return(",".join(map(str, self.limits)))
    
    def lower(self):
        return(self.limits[0])

    def set_lower(self, value):
        self.limits[0] = value

    def bins(self):
        return(self.limits[1])

    def upper(self):
        return(self.limits[2])

    def set_upper(self, value):
        self.limits[2] = value

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

    def is_interactive(self):
        return(self.mode() == "interactive")

    def channels(self):
        if self.options.channels:
            return(self.options.channels)
        else:
            logging.debug("Guessing the number of channels from the file type.")
            guess = guess_channels(self.file_type)
            logging.debug("Got {0} channels.".format(guess))
            return(guess)

    def integer_time(self, float_time):
        "Return the time specified as a float number of milliseconds as "
        "the integer number relevant for the particular resolution of the "
        "file."
        return(int(math.floor(float_time*self.resolution()*1e9)))

    def time_limits(self):
        if self.options.time_limits:
            limits = Limits(self.options.time_limits)
            limits.set_lower(self.integer_time(float(limits.lower())))
            limits.set_upper(self.integer_time(float(limits.upper())))

            return(limits)
        else:
            return(Limits("0,100,{0}".format(self.integer_time(0.01))))

    def pulse_limits(self):
        if self.options.pulse_limits:
            return(Limits(self.options.pulse_limits))
        else:
            return(Limits("0,11,10"))

    def time_distance(self):
        return(self.time_limits().upper())
    
    def pulse_distance(self):
        return(int(self.pulse_limits().upper()))

    def run_intensity(self):
        logging.info("Running an intensity trace.")

        intensity_dst = "{0}.intensity".format(self.filename)
        logging.debug("Intensity trace will be stored at {0}".format(
            intensity_dst))

        if self.mode() == "t2":
            if self.options.intensity_bin_width:
                intensity_bin_width = self.integer_time(
                        self.options.intensity_bin_width)
            else:
                intensity_bin_width = self.integer_time(50)
                
            logging.debug("Intensity bin width: {0} ({1}ms)".format(
                intensity_bin_width, self.options.intensity_bin_width))
        elif self.mode() == "t3":
            if self.options.intensity_bin_width:
                intensity_bin_width = self.options.intensity_bin_width
            else:
                intensity_bin_width = 50000
                
            logging.debug("Intensity bin width: {0} pulses.".format(
                intensity_bin_width))
        else:
            raise(TypeError("Intensity trace for mode {0} could not "
                            "be handled.".format(self.mode())))

        intensity_cmd = [intensity,
                        "--bin-width", str(intensity_bin_width),
                        "--channels", str(self.channels()),
                        "--file-out", intensity_dst,
                        "--mode", self.mode()]

        logging.debug("Intensity command: {0}".format(" ".join(intensity_cmd)))
        
        data = subprocess.Popen(self.data_cmd, stdout=subprocess.PIPE)        
        subprocess.Popen(intensity_cmd, stdin=data.stdout).wait()
    
    def run_correlation(self):
        logging.info("Running correlation.")
        # Distinct states: t2/t3, t3_g1
        if self.options.order:
            order = self.options.order
        else:
            order = 2

        correlate_cmd = []
        histogram_cmd = []
        
        if not self.is_tttr():
            raise(TypeError("{0} is not a tttr mode.".format(self.mode())))
        if self.mode() == "t2":
            logging.debug("Handling t2 correlation of order {0}.".format(order))
            time_distance = str(self.time_distance())
            correlate_cmd = [correlate,
                             "--order", str(order),
                             "--channels", str(self.channels()),
                             "--mode", self.mode(),
                             "--max-time-distance",
                             str(self.time_distance())]
                
            histogram_dst = "{0}.g{1}".format(self.filename, order)
            histogram_cmd = [histogram,
                             "--order", str(order),
                             "--channels", str(self.channels()),
                             "--mode", self.mode(),
                             "--file-out", histogram_dst,
                             "--time", str(self.time_limits())]
        elif self.mode() == "t3":
            if order == 1:
                logging.debug("Histogramming values from a t3 run.")
                histogram_dst = "{0}.g1".format(self.filename)
                histogram_cmd = [histogram,
                                 "--order", str(order),
                                 "--channels", str(self.channels()),
                                 "--mode", self.mode(),
                                 "--file-out", histogram_dst,
                                 "--time", self.time_limits()]
            else:
                logging.debug("Handling t3 correlation of order "
                              "{0}.".format(order))
                time_distance = str(self.time_distance())
                pulse_distance = str(self.pulse_distance())
                correlate_cmd = [correlate,
                                 "--order", str(order),
                                 "--channels", str(self.channels()),
                                 "--mode", self.mode(),
                                 "--max-time-distance",
                                 str(self.time_distance()),
                                 "--max-pulse-distance",
                                 str(self.pulse_distance())]
                    
                histogram_dst = "{0}.g{1}".format(self.filename, order)
                histogram_cmd = [histogram,
                                 "--order", str(order),
                                 "--channels", str(self.channels()),
                                 "--mode", self.mode(),
                                 "--file-out", histogram_dst,
                                 "--time", str(self.time_limits()),
                                 "--pulse", str(self.pulse_limits())]
        else:
            raise(TypeError("Mode could not be handled for "
                            "correlation: {0}".format(self.mode())))

        if correlate_cmd:
            logging.debug("Correlation command: {0} | {1} | {2}".format(
                " ".join(self.data_cmd),
                " ".join(correlate_cmd),
                " ".join(histogram_cmd)))
            data = subprocess.Popen(self.data_cmd, stdout=subprocess.PIPE)
            corr = subprocess.Popen(correlate_cmd,
                                    stdin=data.stdout, stdout=subprocess.PIPE)
            subprocess.Popen(histogram_cmd, stdin=corr.stdout).wait()
        else:
            logging.debug("Correlation command: {0} | {1}".format(
                self.data_cmd, histogram_cmd))
            data = subprocess.Popen(self.data_cmd, stdout=subprocess.PIPE)
            subprocess.Popen(histogram_cmd, stdin=data.stdout).wait()

    def run_interactive(self):
        logging.info("Processing histograms.")
        histogram_dst = "{0}.hist".format(self.filename)
        histogram_cmd = self.data_cmd + ["--file-out", histogram_dst]
        logging.debug("Histogram command: {0}".format(histogram_cmd))
        subprocess.Popen(histogram_cmd).wait()

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
    parser.add_option("-d", "--time", dest="time_limits",
                      help="Specify the time limits for a histogram run, "
                          "as lower,bins,upper.",
                      action="store")
    parser.add_option("-e", "--pulse", dest="pulse_limits",
                      help="Specify the pulse limits for a histogram run.",
                      action="store")
    

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
    
