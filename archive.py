#!/usr/bin/env python

import os
import tarfile
import zipfile
import bz2
import subprocess
import shutil

base_name = "picoquant"
version = "v1.0"
archive_dir = os.path.join("dist", version)
source_dirs = ["doc", "scripts", "src"]
suffixes = [".py", ".c", ".h", "Makefile", ".tex", ".pdf", "README", ".m"]
archive_base = "{0}-{1}".format(base_name, version)
platforms = ["linux_x86", "linux_x86_64"]
executables = ["picoquant", "correlate", "histogram", "intensity"]

def archive_source(source_dirs, suffixes):
    to_archive = list()
    for source_dir in source_dirs:
        for root, dirs, filenames in os.walk(source_dir):
            for filename in filenames:
                if any(map(lambda suffix: filename.lower().endswith(suffix),
                           suffixes)):
                    to_archive.append(os.path.join(root, filename))

    tar_archive = "{0}.tar.bz2".format(archive_base)
    zip_archive = "{0}.zip".format(archive_base)

    with tarfile.open(os.path.join(archive_dir, tar_archive),
                      "w:bz2") as tar_file:
        with zipfile.ZipFile(os.path.join(archive_dir, zip_archive),
                             "w") as zip_file:
            for filename in to_archive:
                tar_file.add(filename)
                zip_file.write(filename)

def make_executables(targets):
    host_uname = os.uname()
    platform = "{0}_{1}".format(host_uname[0].lower(), host_uname[4])
    for target in targets:
        if (target == "linux_x86" and target == platform) or \
           (target == "linux_x86_64" and target == platform):
            archive = os.path.join(archive_dir,
                                   "{0}-{1}-{2}.tar.bz2".format(base_name,
                                                                target,
                                                                version))

            with tarfile.open(archive, "w:bz2") as tar_file:
                os.chdir("scripts")
                tar_file.add("gn.py")
                os.chdir(os.pardir)
 
                os.chdir("src")
                subprocess.Popen(["make", "clean"]).wait()
                subprocess.Popen(["make"]).wait()
                for executable in executables:
                    tar_file.add(executable)
                subprocess.Popen(["make", "clean"]).wait()

                os.chdir(os.pardir)                                         
            

if __name__ == "__main__":
    try:
        os.makedirs(archive_dir)
    except OSError as error:
        if not os.path.isdir(archive_dir):
            raise(error)
        
    archive_source(source_dirs, suffixes)
    make_executables(targets=platforms)
