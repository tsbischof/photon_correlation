import os
import subprocess

from setuptools import setup, find_packages

binaries = [os.path.join("bin", x) for x in
                ["bin_intensity",
                 "channels",
                 "correlate",
                 "gn",
                 "histogram",
                 "intensity",
                 "intensity_to_t2",
                 "picoquant",
                 "t3_as_t2"]]

if any(map(lambda x: not os.path.isfile(x), binaries)):
    subprocess.Popen(["scons",
                      "install"]).wait()

setup(
    name="photon_correlation",
    version="1.6",
    author="Thomas Bischof",
    author_email="tbischof@mit.edu",
    url="",
    packages=find_packages(),
    data_files=[("bin", binaries)])
    
