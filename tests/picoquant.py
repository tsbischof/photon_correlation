#!/usr/bin/env python3

import subprocess
import os

class Picoquant(object):
    def __init__(self, filename):
        self.filename = filename
        self.cmd_base = ["picoquant", "--file-in", filename]

    def compare_to(self, reference_filename,
                   header_only=False, resolution_only=False):
        with open(reference_filename, "r") as reference:
            if header_only:
                line_pairs = zip(self.header(), reference)
            elif resolution_only:
                line_pairs = zip(self.resolution(), reference)
            else:
                line_pairs = zip(self.data(), reference)
                
            for index, lines in enumerate(line_pairs):
                test_line, ref_line = lines
                if test_line.decode().strip() != ref_line.strip():
                    raise(ValueError(r"Failed test at line {0}: "
                                     "'{1}' != '{2}'".format(
                                         index, test_line, ref_line)))
    def header(self):
        cmd = self.cmd_base + ["--header-only"]
        streamer = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        return(streamer.stdout)

    def header_reference(self):
        return("{0}.header".format(self.filename))

    def resolution(self):
        cmd = self.cmd_base + ["--resolution-only"]
        streamer = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        return(streamer.stdout)

    def resolution_reference(self):
        return("{0}.resolution".format(self.filename))

    def data(self):
        cmd = self.cmd_base
        streamer = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        return(streamer.stdout)

    def data_reference(self):
        return("{0}.data".format(self.filename))

    def to_file(self, header_only=False, resolution_only=False):
        if header_only:
            streamer = self.header()
            dst_filename = self.header_reference()
        elif resolution_only:
            streamer = self.resolution()
            dst_filename = self.resolution_reference()
        else:
            streamer = self.data()
            dst_filename = self.data_reference()

        if not os.path.isfile(dst_filename):
            with open(dst_filename, "w") as dst:
                for line in streamer:
                    dst.write(line)

    def to_files(self):
        self.to_file()
        self.to_file(header_only=True)
        self.to_file(resolution_only=True)

    def tests(self):
        try:
            print("Testing header.")
            self.compare_to(self.header_reference(), header_only=True)
        except ValueError as error:
            print("Failed while testing header: {0}".format(error))

        try:
            print("Testing resolution.")
            self.compare_to(self.resolution_reference(), resolution_only=True)
        except ValueError as error:
            print("Failed while testing resolution: {0}".format(error))

        try:
            print("Testing data.")
            self.compare_to(self.data_reference())
        except ValueError as error:
            print("Failed while testing data: {0}".format(error))        


if __name__ == "__main__":
    suffixes = ["hhd", "ht2", "ht3", "phd", "pt2", "pt3", "thd", "thc", "t3r"]
    sample_data_dir = "../sample_data"
    test_files = list()

    for root, dirs, files in os.walk(sample_data_dir):
        for filename in files:
            if any(map(lambda x: filename.endswith(x), suffixes)):
                test_files.append(os.path.join(root, filename))

    for test_file in test_files:
        print(test_file)
        tester = Picoquant(test_file)
        tester.tests()
##        tester.to_files()
                      

