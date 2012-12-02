import sys
import time
import copy
import os

env = Environment()

env["CC"] = "clang"
env["LINK"] = "clang"

version_time = time.strftime("%Y.%m.%d %H.%M")
env["CCFLAGS"] = [
    "-Wall",
    "--pedantic",
    "-D_FILE_OFFSET_BITS=64", # 64-bit file compatibility
    "-DVERSION='1.6'",
    "-DVERSION_STRING='({1})'".format(
        version_time, sys.platform)]
env["LINKFLAGS"] = ["-lm"]

build_dir = "build"
bin_dir = os.path.join(os.path.expanduser("~"), "bin")
lib_dir = os.path.join(os.path.expanduser("~"), "lib")
include_dir = os.path.join(os.path.expanduser("~"),
                           "include",
                           "photon_correlation")

env.Alias("install", bin_dir)
env.Alias("install", lib_dir)
env.Alias("install", include_dir)

SConscript(os.path.join("src", "SConscript"),
           variant_dir=build_dir,
           exports={"env": env,
                    "bin_dir": bin_dir,
                    "lib_dir": lib_dir,
                    "include_dir": include_dir})
