from picoquant import histogram, modes

def lifetime(photons, time_limits, filename=None):
    return(histogram.Histogram(photons,
                               order=1,
                               filename=filename))

if __name__ == "__main__":
    import picoquant

    p = picoquant.Picoquant("v20.pt3")

    my_lifetime = lifetime(p,
                           histogram.Limits(lower=0,
                                            upper=1000000,
                                            bins=10),
                           filename="blargh.g1")

        
