import subprocess

from photon_correlation import histogram, modes, photon

def lifetime(photons, time_limits, filename=None):
##    if not photons.mode == modes.T3:
##        raise(AttributeError("Photon stream must be t3 mode."))
##
##    histogram_cmd = ["histogram",
##                     "--mode", "t3",
##                     "--order", "1",
##                     "--time", str(time_limits)]
##
##    if filename:
##        histogram_cmd.extend(["--file-out", filename])
##        histogram = subprocess.Popen(
##            histogram_cmd,
##            stdin=subprocess.PIPE)
##        for my_photon in photon.byte_stream(photons):
##            histogram.stdin.write(my_photon)
##    else:
##        histogram = subprocess.Popen(
##            histogram_cmd,
##            stdin=subprocess.PIPE,
##            stdout=subprocess.PIPE)
##        for my_photon in photon.byte_stream(photons):
##            histogram.stdin.write(my_photon)
##
##        return(histogram.communicate()[0])
        
    return(histogram.Histogram(photons,
                               order=1,
                               time_limits=time_limits,
                               filename=filename))

def time_dependent_lifetime(photons, time_limits, pulse_bin, filename=None):
    stream = photon.WindowedStream(photons, pulse_bin=pulse_bin)

    for index, window in enumerate(stream):
        limits, my_photons = window
        my_lifetime = lifetime(my_photons,
                               time_limits=time_limits,
                               filename="{0}.g1.{1:020d}_{2:020d}".format(
                                   filename,
                                   limits.lower,
                                   limits.upper))

        yield(limits, my_lifetime.bins())

if __name__ == "__main__":
    import picoquant
    import logging

    logging.basicConfig(level=logging.INFO)

    filename = "v20.pt3"
    pulse_bin = int(10**5)
    time_limits = histogram.Limits(0,
                                   4096,
                                   int(2**12)*256-1)

    photons = picoquant.Picoquant("v20.pt3",
                            print_every=10000)

    for limits, bins in time_dependent_lifetime(photons,
                                                time_limits,
                                                pulse_bin,
                                                filename=filename):
        pass
