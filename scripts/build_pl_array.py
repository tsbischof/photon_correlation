#!/usr/bin/env python3

import csv
import re
import optparse
import logging 
import os
import getpass
import operator

#from picoquant import histogram

def rot90(LoL):
    for i in range(len(LoL[0])):
        yield(map(operator.itemgetter(i), LoL))

def get_filenames(name_base, as_user=getpass.getuser()):
    root_dir, name = os.path.split(name_base)

    if not root_dir:
        root_dir = os.getcwd()

    logging.debug("Searching directory: {0}".format(root_dir))

    if as_user == "rcorrea":
        key = "number"
        gn_parser = re.compile("(?P<name>.+)\.g[0-9]{1}\.(?P<number>[0-9]{8})$")
    else:
        key = "time_start"
        gn_parser = re.compile("(?P<name>.+)\.g[0-9]{1}\."
                               "(?P<time_start>[0-9]{20})_"
                               "(?P<time_end>[0-9]{20})$")

    return(
        sorted(
            filter(lambda x: gn_parser.search(x),
                   filter(lambda y: name in y,
                          os.listdir(root_dir))),
            key=lambda x: gn_parser.search(x).group(key)))

def get_data(filename, strip_autocorrelations):
    with open(filename) as stream:
        for line in csv.reader(stream):
            if len(line) == 4:
                # Lifetime
                yield(line)
            elif len(line) == 5:
                # g2
                channel_0, channel_1, bin_left, bin_right, counts =\
                           line
                if strip_autocorrelations and not (
                      int(channel_0) < int(channel_1)):
                    pass
                else:
                    yield(line)
            else:
                # gn, so in principle we should deal with the autocorrelations,
                # but we will not here
                yield(line)

def gather_histograms(name_base, strip_autocorrelations, as_user):
    filenames = list(get_filenames(name_base, as_user))

    try:
        first = filenames[0]
        logging.debug("Found {0} files to process.".format(len(filenames)))
    except:
        logging.error("No files to process for {0}.".format(name_base))
        return(False)
    
    dst_filename = "{0}.run".format(name_base)
    logging.debug("Writing data to {0}".format(dst_filename))
    with open(dst_filename, "w") as dst_file:
        writer = csv.writer(dst_file)
        for filename in filenames:
            logging.debug("Processing {0}".format(filename))
            data = list(get_data(filename, strip_autocorrelations))
            
            row = list(map(operator.itemgetter(-1), data))

            if filename == first:
                for i in range(len(data[0])-1):
                    writer.writerow([""] + list(map(operator.itemgetter(i),
                                                      data)))

            writer.writerow([filename] + list(row))
        

if __name__ == "__main__":
    usage = "build_pl_array.py name_base"

    parser = optparse.OptionParser(usage=usage)

    parser.add_option("-a", "--auto", dest="strip_autocorrelations",
                      help="Keep the autocorrelations in the histograms.",
                      action="store_false", default=True)
    parser.add_option("-u", "--as-user", dest="as_user",
                      help="Run the program as though it is as the specified"
                           "user.",
                      action="store", type=str, default=getpass.getuser())
    parser.add_option("-V", "--verbose", dest="verbose",
                      help="Debug-level messages.",
                      action="store_true", default=False)

    (options, args) = parser.parse_args()

    if options.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    for root_dir in args:
        gather_histograms(root_dir, options.strip_autocorrelations,
                          options.as_user)
 
