#!/usr/bin/env python3

import csv
import re
import argparse
import logging 
import os
import operator
import configparser

def rot90(LoL):
    for i in range(len(LoL[0])):
        yield(map(operator.itemgetter(i), LoL))

def get_filenames(run_dir):
    logging.debug("Searching directory: {0}".format(run_dir))

    key = "time_start"
    gn_parser = re.compile("(?P<name>.+)\.g[0-9]{1}\."
                           "(?P<time_start>[0-9]{20})_"
                           "(?P<time_end>[0-9]{20})$")

    return(
        map(lambda y: os.path.join(run_dir,
                                   y),
            sorted(
                filter(lambda x: gn_parser.search(x), os.listdir(run_dir)),
                key=lambda x: gn_parser.search(x).group(key))))

def read_data(filename):
    with open(filename) as stream_in:
        for line in csv.reader(stream_in):
            yield(line)

def gather(run_dir):
    # First, determine what sort of run we have:
    # - t2 or t3?
    # - time dependent?
    files = os.listdir(run_dir)

    options = configparser.ConfigParser()
    options.read(os.path.join(run_dir, "options"))

    if "3" in options.get("general", "mode"):
        mode = "t3"
    else:
        mode = "t2"

    order = int(options.get("general", "order"))
    time_dependent = (int(options.get("intensity", "bin_width")) != 0)

    if time_dependent:
        runs = ["bin_intensity"]
        runs.append("g{0}".format(order))

        if mode == "t3":
            runs.append("number")

        for run_type in runs:
            # check that the run actually exists
            run_files = list(
                map(lambda y: os.path.join(run_dir, y),
                    sorted(filter(lambda x: run_type in x and not "td" in x,
                                  files))))

            if not run_files:
                continue

            gather_run_type(os.path.join(run_dir,
                                         "{0}.td".format(run_type)),
                            run_files)

def gather_run_type(dst, files):
    with open(dst, "w") as stream_out:
        writer = csv.writer(stream_out)

        time_parser = re.compile("\.(?P<start>[0-9]+)_(?P<stop>[0-9]+)$")

        start = list()
        stop = list()
        for filename in files:
            parsed = time_parser.search(filename)
            start.append(parsed.group("start"))
            stop.append(parsed.group("stop"))

        readers = list(map(read_data, files))
        first_row = list(map(lambda x: next(x), readers))

        spacers = len(first_row[0][:-1])

        writer.writerow([""]*spacers + start)
        writer.writerow([""]*spacers +  stop)
        my_bin = first_row[0][:-1]
        my_data = list(map(lambda x: x[-1], first_row))
        writer.writerow(my_bin + my_data)

        try:
            while True:
                row = list(map(lambda x: next(x), readers))
                my_bin = row[0][:-1]
                my_data = list(map(lambda x: x[-1], row))
    
                writer.writerow(my_bin + my_data)
        except:
            pass
                
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Gather data from a gn run.")

    parser.add_argument("run_dirs", type=str, nargs="*",
                        help="Run directories to gather.")

    args = parser.parse_args()

    for run_dir in args.run_dirs:
        gather(run_dir)
 
