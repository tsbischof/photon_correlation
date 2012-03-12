#!/usr/bin/env python3

import subprocess
import os

class Histogram(object):
    def __init__(self, filename):
        self.filename = filename
        self.cmd_base = ["histogram", "--file-in", filename]

if __name__ == "__main__":
    suffixes = [""]
    sample_data_dir = "../sample_data/histogram"
    test_files = list()

    for root, dirs, files in os.walk(sample_data_dir):
        for filename in files:
            if any(map(lambda x: filename.endswith(x), suffixes)):
                test_files.append(os.path.join(root, filename))

    for test_file in test_files:
        print(test_file)
        tester = Histogram(test_file)
##        tester.tests()
##        tester.to_files()
                      

