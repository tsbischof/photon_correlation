#!/usr/bin/env python

import optparse
import subprocess
import csv

import numpy
import matplotlib.pyplot as plt

intensity = "intensity"
picoquant = "picoquant"
t2 = "t2"
t3 = "t3"

def ms_to_ps(x):
    return(int(x*10**9))

def apply_threshold(elem, threshold):
    if elem < threshold:
        return(0)
    else:
        return(1)

def on_off_times(stream, channels, threshold):
    first_flip_seen = [False for i in range(channels)]
    since_last_flip = [1 for i in range(channels)]

    threshold_stream = iter(
        map(lambda line:
            map(lambda elem: apply_threshold(elem, threshold),
                line),
            stream))
    current_state = next(threshold_stream)

    for line in threshold_stream:
        for channel in range(channels):
            if line[channel] != current_state[channel]:
                if first_flip_seen[channel]:
                    # We do not want to count the first one, since we do
                    # not know when it started
                    yield(channel, current_state[channel],
                          since_last_flip[channel])
                            
                first_flip_seen[channel] = True
                since_last_flip[channel] = 1
                current_state[channel] = line[channel]
            else:
                since_last_flip[channel] += 1

def intensity_stream(filename, mode, channels, bin_width):
    if mode == t2:
        my_bin_width = ms_to_ps(bin_width)
    else:
        my_bin_width = int(bin_width)

    picoquant_cmd = [picoquant,
                     "--file-in", filename]
    intensity_cmd = [intensity,
                     "--bin-width", str(my_bin_width),
                     "--mode", mode,
                     "--channels", str(channels)]

    data_stream = subprocess.Popen(picoquant_cmd,
                                   stdout=subprocess.PIPE)
    bin_stream = subprocess.Popen(intensity_cmd,
                                  stdin=data_stream.stdout,
                                  stdout=subprocess.PIPE)

    for line in csv.reader(bin_stream.stdout):
        channel_counts = map(int, line[2:])
        intensities = map(lambda counts: float(counts)/bin_width,
                          channel_counts)

        yield(intensities)

def blinking(filename, mode, channels, bin_width, threshold):
    print(filename)

    intensities = intensity_stream(filename, mode, channels, bin_width)

    all_events = dict()
    for channel in range(channels):
        all_events[channel] = {0: list(), 1: list()}
        
    for channel, state, duration in \
        on_off_times(intensities, channels, threshold):
        all_events[channel][state].append(duration)

    # Things you can do:
    # - plot histograms
##    for channel, events in all_events.items():
##        for state, durations in events.items():
##            plt.clf()
##            plt.hist(durations)
##            plt.xlabel("Duration")
##            plt.ylabel("Counts")
##            plt.show()

    return(all_events)
        

if __name__ == "__main__":
    parser = optparse.OptionParser()
    parser.add_option("-m", "--mode", dest="mode",
                      help="Mode to interpret the data as. By default, "
                          "the program will attempt to guess the mode.",
                      action="store")
    parser.add_option("-w", "--bin-width", dest="bin_width",
                      help="Set the bin width for the intensity run, "
                           "in ms (t2) or pulses (t3).",
                      action="store", type=float, default=50)
    parser.add_option("-c", "--channels", dest="channels",
                      help="Number of channels in the data. The default is "
                          "to guess the number from the file type.",
                      action="store", type=int, default=2)
    parser.add_option("-H", "--threshold", dest="threshold",
                      help="Threshold rate of photon arrival for blinking "
                           "analysis, in counts per second.",
                      action="store", type=float)

    (options, args) = parser.parse_args()

    channels = options.channels
    if options.mode:
        mode = options.mode.lower()
    else:
        raise(ValueError("Must specify a mode."))

    bin_width = options.bin_width

    if options.threshold:
        threshold = options.threshold
    else:
        raise(ValueError("Must specify a threshold."))
    
    for filename in args:
        blinking(filename, mode, channels, bin_width, threshold)
