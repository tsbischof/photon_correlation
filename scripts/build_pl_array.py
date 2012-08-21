#!/usr/bin/env python3

import csv
import re
import optparse
import logging 
import os

#from picoquant import histogram

def gather_histograms(root_dir, strip_autocorrelations):
    g2_parser = re.compile("(?P<name>[^\.]+)\.g2\.(?P<number>[0-9]+)$")

    filenames = list()

    for filename in sorted(
        filter(lambda x: g2_parser.search(x):
        
        print(filename)

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    
    usage = "build_pl_array.py directory"

    parser = optparse.OptionParser(usage=usage)

    parser.add_option("-a", "--no-auto", dest="strip_autocorrelations",
                      help="Remove the autocorrelations from the histograms.",
                      action="store_false", default=True)

    (options, args) = parser.parse_args()

    for root_dir in args:
        gather_histograms(root_dir, options.strip_autocorrelations)
 
