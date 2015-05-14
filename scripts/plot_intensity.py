#!/usr/bin/env python3

import csv
import sys
import argparse

import matplotlib.pyplot as plt

def intensity_from_stream(stream):
    for line in csv.reader(stream):
        time_left = int(line[0])
        time_right = int(line[1])
        counts = map(int, line[2:])
        yield(((time_left, time_right), counts))

def plot_intensity(intensity, mode="t2"):
    plt.clf()

    if mode == "t2":
        times = list(map(lambda x: float(x[0][0])/1e12, intensity))
        counts = list(map(
            lambda x: list(map(
                lambda y: float(y)/(x[0][1]-x[0][0])*10**12,
                x[1])),
            intensity))

        for i in range(len(counts[0])):
            plt.plot(times,
                     list(map(lambda x: x[i], counts)),
                     label=str(i))
             
        plt.xlabel("Time/s")
        plt.ylabel("PL intensity/(counts/second)")
    elif mode == "t3":
        times = list(map(lambda x: float(x[0][0]), intensity))
        counts = list(map(
            lambda x: list(map(
                lambda y: float(y)/(x[0][1]-x[0][0]),
                x[1])),
            intensity))

        for i in range(len(counts[0])):
            plt.plot(times,
                     list(map(lambda x: x[i], counts)),
                     label=str(i))
             
        plt.xlabel("Pulse number")
        plt.ylabel("PL intensity/(counts/pulse)")
    else:
        raise(ValueError("Unknown mode: {0}".format(mode)))
    

    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot an intensity.")

    parser.add_argument("--mode", default="t2", type=str,
                        help="Mode of the photons, either t2 or t3.")

    parser.add_argument("files", type=str, nargs="*",
                        help="Filenames containing g2 data to plot.")

    args = parser.parse_args()

    for filename in args.files:
        with open(filename) as stream_in:
            intensity = list(intensity_from_stream(stream_in))

        plot_intensity(intensity, mode=args.mode)
