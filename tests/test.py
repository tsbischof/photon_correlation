def diff(old, new):
    oiter = iter(old)
    niter = iter(new)
    
    while True:
        try:
            o = next(oiter)
        except StopIteration:
            try:
                n = next(niter)
                return(False)
            except StopIteration:
                return(True)

        try:
            n = next(niter)
        except StopIteration:
            return(False)

        if o != n:
            return(False)
        
