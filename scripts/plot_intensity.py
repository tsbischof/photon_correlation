#!/usr/bin/env python

import sys
import csv
import matplotlib.pyplot as plt
import numpy

if __name__ == "__main__":
    times = list()
    intensities = None
    for line in sys.stdin:
        my_line = line.split(",")
        times.append(int(my_line[0]))
        
        my_intensities = my_line[1:]
        if not intensities:
            intensities = [list() for i in my_intensities]

        for index, intensity in enumerate(my_intensities):
            intensities[index].append(int(intensity))

    plt.clf()
    for index, intensity in enumerate(intensities):
        plt.plot(times, intensity)

    plt.xlabel("Time")
    plt.ylabel("Counts")
    plt.show()
        
            

        
