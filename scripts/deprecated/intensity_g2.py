#!/usr/bin/env python3

import argparse
import logging
import subprocess
import operator

import sys
sys.path.append("/home/tsbischof/src/photon_correlation")

import photon_correlation as pc

def mean(L):
    return(sum(L)/len(L))

def correlate_g2(filename_in, filename_out, mode, limits, channels):
    bin_width = (limits.upper - limits.lower) // limits.bins   
    intensity_filename = "{0}.intensity".format(filename_in)
    
    logging.info("Calculating intensity.")
    photons = subprocess.Popen(["picoquant",
                                "--file-in", filename_in],
                               stdout=subprocess.PIPE)
    intensity = subprocess.Popen(["photon_intensity",
                                  "--mode", mode,
                                  "--channels", str(channels),
                                  "--bin-width", str(bin_width)],
                                 stdin=photons.stdout,
                                 stdout=subprocess.PIPE)

    my_intensity = pc.Intensity().from_stream(map(lambda x: x.decode(),
                                                  intensity.stdout))

    logging.info("Calculating channel intensities.")
    v0 = list(map(operator.itemgetter(0), my_intensity.bins))
    v1 = list(map(operator.itemgetter(1), my_intensity.bins))

    logging.info("Calculating correlation.")
    print(correlate(v0, v0, limits.bins))
    
def correlate(vector0, vector1, limit):
    steps = len(vector0) - limit
    result = [0 for i in range(limit)]
    if steps > 0:
        for i in range(len(vector0) - limit):
            for j in range(limit):
                result[j] += vector0[i]*vector1[i+j]

        return(list(map(lambda x: x/(steps*mean(vector0)*mean(vector1)),
                        result)))
    else:
        return(result)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    
    parser = argparse.ArgumentParser(
        prog="intensity_g2",
        description="Correlate photons by integration into a "
        "time-dependent vector.")

    parser.add_argument("--mode", type=str,
                        default="t2",
                        help="Mode of the photons to be correlated. Either "
                        "t2 or t3.")
    parser.add_argument("--limits", type=str,
                        help="Limits of the correlation, either in time (t2) "
                        "or nmber of pulses (t3)")
    parser.add_argument("--file-in", type=str,
                        help="Filename in. By default, the output becomes "
                        "filename.g2")
    parser.add_argument("--file-out", type=str,
                        help="Filename out. By default, this is based on the "
                        "input filename")
    parser.add_argument("--channels", type=int,
                        default=2,
                        help="Number of channels in the photon stream.")

    args = parser.parse_args()

    if args.file_in is None:
        print("Must specify input file.")
        exit(-1)
    else:
        args.file_in = args.file_in

    if args.file_out is None:
        args.file_out = "{0}.g2".format(args.file_in)
    else:
        args.file_out = args.file_out

    if not (args.mode == "t2" or args.mode == "t3"):
        print(args.mode)
        print("Mode must be t2 or t3")
        exit(-1)

    try:
        limits = pc.Limits().from_string(args.limits)
    except Exception as error:
        print("Invalid limits: {0}".format(args.limits))
        print("Error given: {0}".format(error))
        exit(-1)

    correlate_g2(args.file_in, args.file_out, args.mode, limits, args.channels)
    
