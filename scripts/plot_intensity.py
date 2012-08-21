#!/usr/bin/env python

import csv
import sys

import matplotlib.pyplot as plt

def intensity_from_stream(stream):
    for line in stream:
        time_left = int(line[0])
        time_right = int(line[1])
        counts = map(int, line[2:])
        yield(((time_left, time_right), counts))

def plot_intensity(intensity):
    plt.clf()

    plt.plot(map(lambda x: float(x[0][0])/10**12, intensity),
             map(lambda x: list(map(lambda y: float(y)/(x[0][1]-x[0][0])*10**12, x[1])), intensity))
##             map(lambda x: [float(x[1][0])/x[1][0], float(x[1][1])/x[1][0]],
##                 intensity))
    plt.xlabel("Time/s")
    plt.ylabel("PL intensity/(counts/second)")
    plt.show()

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        with open(filename) as stream:
            intensity = list(intensity_from_stream(csv.reader(stream)))

        plot_intensity(intensity)
