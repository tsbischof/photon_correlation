from .Photon import Photon

class T3(Photon):
    def __init__(self, channel=None, pulse=None, time=None,
                 string=None):
        if string is not None:
            self.from_string(string)
        else:
            self.channel = channel
            self.pulse = pulse
            self.time = time

    def __repr__(self):
        return(",".join(map(str, (self.channel, self.pulse, self.time))))

    def __eq__(self, other):
        return(self.channel == other.channel and \
               self.pulse == other.pulse and \
               self.time == other.time)

    def __lt__(self, other):
        if self.pulse == other.pulse:
            return(self.time < other.time)
        else:
            return(self.pulse < other.pulse)

    def __le__(self, other):
        if self.pulse == other.pulse:
            return(self.time <= other.time)
        else:
            return(self.pulse <= other.pulse)

    def __gt__(self, other):
        if self.pulse == other.pulse:
            return(self.time > other.time)
        else:
            return(self.pulse > other.pulse)

    def __ge__(self, other):
        if self.pulse == other.pulse:
            return(self.time >= other.time)
        else:
            return(self.pulse >= other.pulse)

    @property
    def window_dimension(self):
        return(self.pulse)

    def from_string(self, string):
        self.channel, self.pulse, self.time = map(int, string.split(","))
