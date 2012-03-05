import subprocess
import os
import csv

exec_dir = "/home/tsbischof/bin"

class Picoquant(object):
    def __init__(self, filename):
        self.filename = filename
        self.data = subprocess.Popen([os.path.join(exec_dir, "picoquant"),
                                      "--file-in", self.filename],
                                     stdout=subprocess.PIPE)

    def stream(self):
        for line in self.data.stdout:
            yield(line.decode().strip().split(","))

if __name__ == "__main__":
    data = list()
    for line in Picoquant("../../sample_data/picoharp/v20.pt2").stream():
        data.append(line)

    print(data[:100])

