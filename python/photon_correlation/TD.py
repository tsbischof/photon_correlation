import csv
import os
import logging
import bisect
import itertools
import re
import datetime
import bz2

from Intensity import *

def read_td(filename):
    if filename.endswith("bz2"):
        stream_in = bz2.BZ2File(filename)
    else:
        stream_in = open(filename)
    
    header = list()
    yielded_header = False
    for line in csv.reader(stream_in):
        if not yielded_header:
            if not line[0]:
                header.append(line[2:])
            else:
                yield(header)
                yielded_header = True

        if yielded_header:
            yield((float(line[0]), float(line[1])),
                  tuple(map(int, line[2:])))

    stream_in.close()

class TD(object):
    def __init__(self, run_dir=None, filename=None, order=None):
        self.parser = parser = re.compile("g(?P<order>[0-9])\.td(\.bz2)?$")
        if run_dir is None and filename is None:
            raise(ValueError("Must specify a run directory or filename."))
        elif not run_dir is None and not filename is None:
            self.run_dir = run_dir
            self.filename = filename
        elif not run_dir is None:
            self.run_dir = run_dir

            matches = list(filter(self.parser.search, os.listdir(self.run_dir)))
            
            if len(matches) == 1:
                self.filename = matches[0]
            else:
                if len(matches) == 0:
                    raise(ValueError("Could not find the td file "
                                     "in {}".format(self.run_dir)))
                else:
                    raise(ValueError("Found {} possible td files. "
                                     "Specify the correct one "
                                     "explicitly".format(len(matches))))
        else:
            self.run_dir, self.filename = os.path.split(filename)
        
        self._intensity = None

    def intensity(self):
        if not self._intensity:
            self._intensity = Intensity(os.path.join(self.run_dir,
                                                     "intensity"))

        return(self._intensity)

    def max_intensity(self):
        return(max(self.intensity().summed()[0]))

    def gn_td(self, filename=None):
        if not filename:
            filename = self.gn_filename()

        return(read_td(filename))        

    def gn_filename(self):
        return(os.path.join(self.run_dir, self.filename))

    def coarse_binning(self, n_bins=20):
        bins = list(map(lambda x: float(x)/n_bins, range(n_bins+1)))
        dst_filename = "{0}.coarse.{1}".format(
            re.sub("\.bz2$", "", self.gn_filename()),
            n_bins)

        self.bin(bins, dst_filename)

        return(TD(filename=dst_filename))

    def bin(self, magnitude_bins, dst_filename):
        """
        Take the raw gn and bin it by intensity.
        """
        if not os.path.exists(dst_filename):
            logging.info("Binning to {}".format(dst_filename))

            max_intensity = self.max_intensity()

            logging.debug("Max intensity: {}".format(max_intensity))
            
            # These define the values we will use to
            # distribute windows by their intensity
            intensity_bins = list(map(lambda x: x*max_intensity,
                                      magnitude_bins))

            binned = [None for i in range(len(intensity_bins)-1)]

            # The heavy lifting. Go through each
            gn = self.gn_td()
            header = next(gn)
            
            for index, vals in enumerate(\
                itertools.izip(gn, self.intensity().stream())):
                gn_line, intensity_line = vals
                
                gn_window, gn_vals = gn_line
                intensity_window, intensity_vals = intensity_line

                intensity = sum(intensity_vals)

                bin_index = bisect.bisect_left(intensity_bins, intensity) - 1
                if bin_index < 0:
                    bin_index = 0

                if index % 100 == 0:
                    logging.info("{}: {}: {}, {}".format(
                        datetime.datetime.now(),
                        gn_window,
                        intensity,
                        bin_index))

                if binned[bin_index] is None:
                    binned[bin_index] = numpy.array(gn_vals)
                else:
                    binned[bin_index] += numpy.array(gn_vals)

            logging.info("Writing to {}".format(dst_filename))

            with open(dst_filename, "w") as stream_out:
                writer = csv.writer(stream_out)

                for line in header:
                    writer.writerow(["", ""] + line)

                for i in range(len(intensity_bins)-1):
                    vals = binned[i]

                    if vals is None:
                        vals = [0 for j in range(len(header[0]))]

                    line = [intensity_bins[i], intensity_bins[i+1]] + list(vals)
                    
                    writer.writerow(list(map(str, line)))

    def sum_magnitude(self, min_magnitude, max_magnitude):
        """
        Treats the windows as intensities, and sums all entries whose
        intensities fall within the speicifed limits.
        """
        max_intensity = self.max_intensity()

        td = self.gn_td()
        bins = next(td)

        counts = None
        
        for window, vals in td:
            left, right = list(map(lambda x: x/max_intensity, window))

            if min_magnitude <= left and right <= max_magnitude:
                if counts is None:
                    counts = numpy.array(vals)
                else:
                    counts += numpy.array(vals)

        if counts is None:
            counts = [0 for i in range(len(bins[0]))]
        
        counts = list(counts)

        return(bins, counts)
        
    def maximum_intensity_gn(self, threshold=0.7):
        """
        Find the top threshold% of intensity bins and use those to accumulate
        data for the maximum gn data.
        """
        coarse = self.coarse_binning(n_bins=20)

        min_magnitude = threshold
        max_magnitude = 1
        
        dst_filename = self.gn_filename() + \
                       ".sum_{}_{}".format(min_magnitude, max_magnitude)

        if not os.path.exists(dst_filename):
            bins, counts = coarse.sum_magnitude(min_magnitude, max_magnitude)

            with open(dst_filename, "w") as stream_out:
                        writer = csv.writer(stream_out)
                        
                        for my_bin, counts in zip(transpose(bins), counts):
                            writer.writerow(list(map(str, my_bin)) +\
                                            [str(counts)])
            
        return(dst_filename)

if __name__ == "__main__":
    td = TD(filename="/home/tsbischof/Documents/papers/inas_exciton_lifecycle_paper/data/td/20130117/qdv1295_633nm_1mhz_dot09.ht3.50000.g1.run/g1.td.bz2")

    td.maximum_intensity_gn(threshold=0.7)
    
