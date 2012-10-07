# Specify the collection modes
MODE_UNKNOWN = None
INTERACTIVE = "interactive"
CONTINUOUS = "continuous"
T2 = "t2"
T3 = "t3"
TTTR = [T2, T3]

# The three timing boxes
PICOHARP = "p"
TIMEHARP = "t"
HYDRAHARP = "h"

# Mapping extensions to modes
MODES = {"phd": INTERACTIVE,
         "pt2": T2,
         "pt3": T3,
         "hhd": INTERACTIVE,
         "ht2": T2,
         "ht3": T3,
         "thd": INTERACTIVE,
         "thc": CONTINUOUS,
         "t3r": T3}

# Guesses for the number of channels in a box
CHANNELS = {PICOHARP: 2,
            HYDRAHARP: 4,
            TIMEHARP: 2}

# Time scales for the histogram
SCALE_LINEAR = "linear"
SCALE_LOG = "log"
SCALE_LOG_ZERO = "log-zero"

# Routines to guess useful parameters
def guess_mode(filename):
    try:
        extension = filename[-3:]
    except:
        return(None)

    return(MODES[extension])

def guess_channels(filename):
    try:
        extension = filename[-3:]
    except:
        return(None)

    return(CHANNELS[extension[0]])
    
