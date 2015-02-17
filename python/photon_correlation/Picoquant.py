try:
    import configparser
    import io
except:
    import StringIO as io
    import ConfigParser as configparser

import subprocess
import csv

class FakeIniSection(object):
    """
    Wrapper to enable configparser to parse a file without sections.
    """
    def __init__(self, fp):
        self.fp = fp
        self.sechead = '[header]\n'
        
    def readline(self):
        if self.sechead:
            try:
                return(self.sechead)
            finally:
                self.sechead = None
        else:
            return(self.fp.readline())

def fake_ini_section(text, section="header"):
    """
    Add the fake section "{}" to an ini-like file.
    """.format(section)
    return("[{}]".format(section) + text)

class Picoquant(object):
    """
    Base class for Picoquant data. This includes:
    1. Common header
    2. Hardware header
    3. Mode header.
    4. Data

    Additionally, various helper routines are used to decode information
    about the files, including resolution and mode.
    """
    def __init__(self, filename):
        self._filename = filename
        self._header = None
        self._resolution = None
        self._data = None
        self._mode = None

    def header(self):
        if not self._header:
            header_raw = subprocess.Popen(
                ["picoquant",
                 "--file-in", self._filename,
                 "--header-only"],
                stdout=subprocess.PIPE).stdout.read().decode()

            self._header = configparser.ConfigParser()
            self._header.readfp(io.StringIO(fake_ini_section(header_raw)))

        return(self._header)

    def repetition_rate(self):
        """
        Use the sync channel (or channel 0, if no sync) to determine
        the repetition rate of the laser.
        """
        sync_rate = None
        if self.header().has_option("header", "syncrate"):
            return(self.header().getfloat("header", "syncrate"))
        else:
            return(self.header().getfloat("header", "inprate[0]"))
    
    def channels(self):
        """
        Return the number of signal channels present in the device.
        """
        return(self.header().getint("header", "inputchannelspresent"))

    def resolution(self):
        if not self._resolution:
            resolution_raw = subprocess.Popen(
                ["picoquant",
                 "--file-in", self._filename,
                 "--resolution-only"],
                stdout=subprocess.PIPE).stdout.read().decode()

            if "," in resolution_raw:
                # curves
                self._resolution = list()
                for curve, resolution in csv.reader(\
                    io.StringIO(resolution_raw)):
                    self._resolution.append((int(curve),
                                             float(resolution)))
                self._resolution = tuple(self._resolution)
            else:
                # single result
                self._resolution = float(resolution_raw)

        return(self._resolution)

    def mode(self):
        if not self._mode:
            self._mode = subprocess.Popen(
                ["picoquant",
                 "--file-in", self._filename,
                 "--mode-only"],
                stdout=subprocess.PIPE).communicate()[0].decode().strip()

        return(self._mode)

    def integration_time(self):
        """
        Return the integration time, in ms.
        """
        return(self.header().getint("header", "stopafter"))

    def __iter__(self):
        data = subprocess.Popen(
            ["picoquant", 
             "--file-in", self._filename],
            stdout=subprocess.PIPE)
        return(csv.reader(data.stdout))

##if __name__ == "__main__":
##    pq = Picoquant("20130116/qdv1295_633nm_1mhz_dot01.ht3")
##    print(pq.header().get_section("header"))
