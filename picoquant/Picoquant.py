import subprocess
import os
import re
import csv

from picoquant import modes
from picoquant import files
from picoquant import photon
from picoquant import interactive

class Picoquant(photon.PhotonStream):
    def __init__(self, filename, channels=None, mode=None, decode=False):
        super(Picoquant, self).__init__()
        if not os.path.isfile(filename):
            raise(OSError("File does not exist: {0}".format(filename)))
        else:
            self.filename = filename
        if mode:
            self.mode = mode
        else:
            self.mode = modes.guess_mode(filename)

        if channels:
            self.channels = channels
        else:
            self.channels = modes.guess_channels(filename)

        self._resolution = None
        self._header = None

        self._stream = None
        self._stdout = None
        self._stderr = None

        self.decode = decode

    def header(self):
        if not self._header:
            stdout, stderr = subprocess.Popen(
                [files.PICOQUANT,
                 "--file-in", self.filename,
                 "--header-only"],
                stdout=subprocess.PIPE).communicate()
            if stderr:
                raise(Exception("Call to picoquant failed: {0}".format(stderr)))

            self._header = dict()
            for line in stdout.decode().split("\n"):
                try:
                    parsed = re.search("(?P<name>.+) = (?P<value>.+)", line)
                    self._header[parsed.group("name")] = parsed.group("value")
                except AttributeError:
                    pass

        return(self._header)

    def resolution(self):
        if not self._resolution:
            stdout, stderr = subprocess.Popen(
                [files.PICOQUANT,
                 "--file-in", self.filename,
                 "--resolution-only"],
                stdout=subprocess.PIPE).communicate()

            if stderr:
                raise(Exception("Call to picoquant failed: {0}".format(stderr)))

            self._resolution = list()
            for line in stdout.decode().split("\n"):
                if self.mode in modes.TTTR:
                    try:
                        raw_value = float(line)

                        if raw_value.is_integer():
                            self._resolution.append(int(raw_value))
                        else:
                            self._resolution.append(raw_value)
                    except ValueError:
                        pass
                else:
                    # Interactive or continuous
                    try:
                        raw_values = next(csv.reader(line.split("\n")))
                        
                        curve = int(raw_values[0])
                        resolution = float(raw_values[1])
                        if resolution.is_integer():
                            self._resolution.append((curve, int(resolution)))
                        else:
                            self._resolution.append((curve, resolution))
                    except:
                        pass

            if len(self._resolution) == 1:
                self._resolution = self._resolution[0]
            else:
                self._resolution = tuple(self._resolution)

        return(self._resolution)

    def __iter__(self):
        return(self)

    def __next__(self):
        if not self._stream:
            self._stdout, self._stderr = subprocess.Popen(
                [files.PICOQUANT,
                 "--file-in", self.filename],
                stdout=subprocess.PIPE,
                bufsize=4096).communicate()
            
            if self.mode in modes.TTTR:
                self._stream = map(lambda x: photon.Photon(mode=self.mode,
                                                           string=x.decode(),
                                                           decode=self.decode),
                                   self._stdout.splitlines())
            elif self.mode == modes.INTERACTIVE:
                self._stream = map(
                    lambda x: interactive.InteractiveBin(string=x.decode()),
                    self._stdout.splitlines())
            else:
                raise(ValueError("Mode not recognized: {0}".format(self.mode)))

        if self._stderr:
            raise(IOError("Call to picoquant failed: {0}".format(
                self._stderr)))

        try:
            return(next(self._stream))
        except StopIteration:
            self._stream = None
            raise(StopIteration)
                

if __name__ == "__main__":
    for filename in ["v20.phd", "v20.pt2"]:
        print(filename)
        p = Picoquant(filename)
        print(p.resolution())
        print(p.header())
        for index, line in zip(range(10), p):
            print(line)
