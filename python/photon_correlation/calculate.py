import csv
import shelve
import os
import subprocess
import re
import math
import logging

from Picoquant import Picoquant
from Limits import Limits
from Lifetime import Lifetime
import TD

from util import *

def picoquant(filename, print_every=100000):
    cmd = ["picoquant", "--file-in", filename]

    if print_every > 0:
        cmd.extend(("--print-every", str(print_every)))

    return(subprocess.Popen(cmd, stdout=subprocess.PIPE))

def intensity(data_filename, dst_filename=None, bin_width=50000,
              mode=None, channels=None, time_offsets=None,
              repetition_rate=None):
    logging.info("Calculating intensity of {} with bin width of {}".format(
        data_filename, bin_width))
    pq = Picoquant(data_filename)

    if dst_filename is None:    
        dst_filename = os.path.join("{}.intensity.run".format(data_filename),
                                    "intensity.{}".format(bin_width))

    root_dir =  os.path.split(dst_filename)[0]

    if not os.path.isdir(root_dir):
        os.makedirs(root_dir)

    if channels is None:
        channels = pq.channels()

    if mode is None:
        mode = pq.mode()

    if repetition_rate is None:
        repetition_rate = pq.repetition_rate()

    photons = picoquant(data_filename)
    if time_offsets is not None:
        photons = apply_t3_time_offsets(photons, time_offsets, repetition_rate)

    intensity_cmd = ["photon_intensity",
                     "--bin-width", str(bin_width),
                     "--mode", mode,
                     "--channels", str(channels),
                     "--file-out", dst_filename]

    subprocess.Popen(intensity_cmd, stdin=photons.stdout).wait()

def apply_t3_time_offsets(photons, time_offsets, repetition_rate):
    if time_offsets.all_same():
        t2 = subprocess.Popen(["photons",
                               "--mode", "t3",
                               "--convert", "t2",
                               "--time-origin", str(time_offsets[0]),
                               "--repetition-rate", str(repetition_rate)],
                              stdin=photons.stdout,
                              stdout=subprocess.PIPE)
        t3 = subprocess.Popen(["photons",
                               "--mode", "t2",
                               "--convert", "t3",
                               "--repetition-rate", str(repetition_rate)],
                              stdin=t2.stdout,
                              stdout=subprocess.PIPE)

    return(t3)

def number_to_channels(photons, correlate=False):
    cmd = ["photon_number_to_channels"]

    if correlate:
        cmd.append("--correlate-successive")

    return(subprocess.Popen(cmd, stdin=photons.stdout, stdout=subprocess.PIPE))

def convert_mode(photons, src_mode, dst_mode, repetition_rate):
    cmd = ["photons",
           "--mode", src_mode,
           "--convert", dst_mode,
           "--repetition-rate", str(repetition_rate)]

    return(subprocess.Popen(cmd,
                            stdin=photons.stdout,
                            stdout=subprocess.PIPE))

def gn(data_filename, dst_dir=None,
       photon_mode=None, gn_mode=None,
       order=2, channels=None,
       time_bins=None, pulse_bins=None,
       repetition_rate=None, time_offsets=None,
       window_width=None, bin_width=None,
       photon_number=False, 
       time_bin_width=1024):
    logging.info("Calculating g{} for {}".format(order, data_filename))

    if not bin_width is None:
        logging.info("This is a time-dependent calculation with a bin "
                     "width of {}".format(bin_width))
    
    if dst_dir is None:
        dst_filename = data_filename
    else:
        dst_filename = os.path.join(dst_dir,
                                    os.path.split(data_filename)[1])

    if not bin_width is None:
        dst_filename += ".{}".format(bin_width)

    pq = Picoquant(data_filename)

    if photon_mode is None:
        photon_mode = pq.mode()

    if photon_mode == "t3":
        if repetition_rate is None:
            repetition_rate = pq.repetition_rate()
        
    if channels is None:
        channels = pq.channels()

    if gn_mode is None:
        gn_mode = photon_mode
        convert = False
    else:
        convert = True
    
    if time_bins is None:
        if gn_mode == "t3" and order == 1:
            # Determine the resolution-limited time bins from the t3 file.
            resolution = int(pq.resolution())

            if data_filename.endswith("ht3"):
                n_bins = 2**15
            else:
                raise(ValueError("Unsupported file type for "
                                 "lifetime: {}".format(t3_filename)))

            time_bins = Limits(0, resolution*n_bins, n_bins=n_bins)
        elif gn_mode == "t3" and order >= 2:
            repetition_time = 1e12/repetition_rate

            bins_per_side = int(math.ceil(repetition_time/time_bin_width))
            n_bins = bins_per_side*2 + 1
            time_bound = bins_per_side*time_bin_width + time_bin_width/2

            time_bins = Limits(-time_bound, time_bound, n_bins=n_bins)
        elif gn_mode == "t2":
            repetition_time = 1e12/repetition_rate

            bins_per_side = int(math.ceil(repetition_time/time_bin_width*1.5))
            n_bins = bins_per_side*2 + 1
            time_bound = bins_per_side*time_bin_width + time_bin_width/2

            time_bins = Limits(-time_bound, time_bound, n_bins=n_bins)
        else:
            raise(ValueError("Unsupported mode for automatic time bins"
                             ": {}".format(mode)))

    if photon_number:
        channels = sum(range(1, channels+1))
        dst_filename += ".number"

    if convert:
        dst_filename += ".{}".format(gn_mode)

    gn_cmd = ["photon_gn",
              "--file-out", dst_filename,
              "--mode", gn_mode,
              "--order", str(order),
              "--time", str(time_bins)]
   
    if gn_mode == "t3" and order > 1:
        if pulse_bins is None:
            pulse_bins = Limits(-1.5, 1.5, n_bins=3)

        gn_cmd.extend(("--pulse", str(pulse_bins)))

    if not bin_width is None:
        gn_cmd.extend(("--bin-width", str(bin_width)))

        if window_width is None:
            gn_cmd.extend(("--window-width", str(bin_width)))

    if not window_width is None:
        gn_cmd.extend(("--window-width", str(window_width)))

    photons = picoquant(data_filename)
    if time_offsets is not None:
        if photon_mode == "t3":
            photons = apply_t3_time_offsets(picoquant(data_filename),
                                            time_offsets, repetition_rate)
        else:
            raise(ValueError("Unsupported mode for time offsets"
                             ": {}".format(photon_mode)))

    if convert:
        # fiddle with time bin defintions
        photons = convert_mode(photons, photon_mode, gn_mode, repetition_rate)
        
    if photon_number:
        photons = number_to_channels(photons)

    gn_cmd.extend(("--channels", str(channels)))
    
    gn = subprocess.Popen(gn_cmd,
                          stdin=photons.stdout).wait()

    if bin_width is not None:
        # In case of time-dependent results, bzip2 the files to save space.
        root, filename = os.path.split(dst_filename)

        for my_root, dirs, files in os.walk(root):
            for filename in files:
                if filename.endswith("td"):
                    dst = os.path.join(my_root, filename)
                    logging.info("Compressing {}".format(dst))
                    subprocess.Popen(["bzip2", dst]).wait()

def intensity_g2(src_filename, dst_filename,
                 channels=None,
                 mode=None, bin_width=None):
    logging.info("Calculating intensity g2 for {} with bin width {}".format(
        src_filename, bin_width))

    pq = Picoquant(src_filename)

    if mode is None:
        mode = pq.mode()

    if channels is None:
        channels = pq.channels()

    photons = picoquant(src_filename)
    
    cmd = ["photon_intensity_correlate",
           "--mode", mode,
           "--bin-width", str(bin_width),
           "--channels", str(channels),
           "--file-out", dst_filename]

    subprocess.Popen(cmd, stdin=photons.stdout).wait()

def flid(src_filename, dst_filename, intensity_bins,
         window_width, time_bins=None, time_offsets=None):
    logging.info("Calculating flid for {} with window width {}".format(
        src_filename, window_width))
    
    pq = Picoquant(src_filename)

    if not pq.mode() == "t3":
        raise(ValueError("T3 mode data expected for flid calculation."))

    if time_bins is None:
        time_bins = Limits(0, int(1e12/pq.repetition_rate()), 1000)
        
    photons = picoquant(src_filename)

    if time_offsets is not None:
        photons = apply_t3_time_offsets(photons,
                                        time_offsets, pq.repetition_rate())
    

    cmd = ["photon_flid",
           "--window-width", str(window_width),
           "--time", str(time_bins),
           "--intensity", str(intensity_bins),
           "--file-out", dst_filename]

    subprocess.Popen(cmd, stdin=photons.stdout).wait()

def idgn(src_filename, dst_filename, intensity_bins,
         order=2, window_width=50000, repetition_rate=None,
         channels=None,
         time_bins=None, pulse_bins=None, mode=None,
         time_bin_width=1024):
    logging.info("Calculating g{} for {}".format(order, src_filename))

    pq = Picoquant(src_filename)

    if mode is None:
        mode = pq.mode()

    if mode == "t3":
        if repetition_rate is None:
            repetition_rate = pq.repetition_rate()
        
    if channels is None:
        channels = pq.channels()
    
    if time_bins is None:
        if mode == "t3" and order == 1:
            # Determine the resolution-limited time bins from the t3 file.
            resolution = int(pq.resolution())

            if data_filename.endswith("ht3"):
                n_bins = 2**15
            else:
                raise(ValueError("Unsupported file type for "
                                 "lifetime: {}".format(t3_filename)))

            time_bins = Limits(0, resolution*n_bins, n_bins=n_bins)
        elif mode == "t3" and order >= 2:
            repetition_time = 1e12/repetition_rate

            bins_per_side = int(math.ceil(repetition_time/time_bin_width))
            n_bins = bins_per_side*2 + 1
            time_bound = bins_per_side*time_bin_width + time_bin_width/2

            time_bins = Limits(-time_bound, time_bound, n_bins=n_bins)
        elif mode == "t2":
            repetition_time = 1e12/repetition_rate

            bins_per_side = int(math.ceil(repetition_time/time_bin_width*1.5))
            n_bins = bins_per_side*2 + 1
            time_bound = bins_per_side*time_bin_width + time_bin_width/2

            time_bins = Limits(-time_bound, time_bound, n_bins=n_bins)
        else:
            raise(ValueError("Unsupported mode for automatic time bins"
                             ": {}".format(mode)))

    gn_cmd = ["photon_intensity_dependent_gn",
              "--file-out", dst_filename,
              "--mode", mode,
              "--order", str(order),
              "--channels", str(channels),
              "--intensity", str(intensity_bins),
              "--window-width", str(window_width),
              "--time", str(time_bins)]
   
    if mode == "t3" and order > 1:
        if pulse_bins is None:
            pulse_bins = Limits(-1.5, 1.5, n_bins=3)

        gn_cmd.extend(("--pulse", str(pulse_bins)))

    photons = picoquant(src_filename)
    gn = subprocess.Popen(gn_cmd, stdin=photons.stdout).wait()
