from .Photon import Photon

class T2(Photon):
    def __init__(self, channel=None, time=None):
        self.channel = channel
        self.time = time

    def __repr__(self):
        return(",".join(map(str, (self.channel, self.time))))

    def __eq__(self, other):
        return(self.channel == other.channel and \
               self.time == other.time)

    def __lt__(self, other):
        return(self.time < other.time)

    def __le__(self, other):
        return(self.time <= other.time)

    def __gt__(self, other):
        return(self.time > other.time)

    def __ge__(self, other):
        return(self.time >= other.time)

    @property
    def window_dimension(self):
        return(self.time)

    def from_string(self, string):
        self.channel, self.time = map(int, string.split(","))
