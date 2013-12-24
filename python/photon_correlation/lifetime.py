import subprocess
import os

from photon_correlation import histogram, modes, photon

def lifetime(photons, time_limits, filename=None):      
    return(histogram.Histogram(photons,
                               order=1,
                               time_limits=time_limits,
                               filename=filename))

def time_dependent_lifetime(photons, time_limits, pulse_bin,
                            run_dir=os.getcwd(), filename=None):
    if not photons.mode == modes.T3:
        raise(TypeError("Photons from {0} are not of t3 "
                        "type.".format(filename)))
    
    stream = photon.WindowedStream(photons, pulse_bin=pulse_bin)
        
    try:
        os.makedirs(run_dir)
    except Exception as error:
        if os.path.isdir(run_dir):
            pass
        else:
            raise(error)

    for index, window in enumerate(stream):
        limits, photons = window
        if filename:
            my_filename = os.path.join(
                run_dir,
                "{0}.g1.{1:020d}_{2:020d}".format(
                    filename,
                    limits.lower,
                    limits.upper))
        else:
            my_filename = None
            
        my_lifetime = lifetime(photons,
                               time_limits=time_limits,
                               filename=my_filename)
        
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
