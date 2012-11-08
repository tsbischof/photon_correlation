import itertools
import sys
import time
import copy
import os

env = Environment()

version_time = time.strftime("%Y.%m.%d %H.%M")
ccflags = ["-Wall",
           "-std=c99",
           "--pedantic",
           "-D_FILE_OFFSET_BITS=64", # 64-bit file compatibility
           "-DVERSION_MAJOR=1",
           "-DVERSION_MINOR=6",
           "-lm"]
ccflags.append(
    "-DVERSION_STRING='{0} ({1})'".format(
        version_time, sys.platform))
env["CCFLAGS"] = ccflags
env["LINKFLAGS"] = ccflags

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

##statuses = ["release", "debug"]
##targets = ["linux"]#, "win"]
##bits = [32, 64]
##
##for status, target, n_bits in itertools.product(statuses, targets, bits):
##    my_env = env
##    my_ccflags = copy.deepcopy(ccflags)
##    if status == "release":
##        my_ccflags.append(["-O3"])
##    else:
##        my_ccflags.append(["-g"])    
##
##    if n_bits == 32:
##        my_ccflags.append(["-m32"])
##    else:
##        my_ccflags.append(["-m64"])
##
##    my_ccflags += ["-DVERSION_STRING='{0} ({1}{2})'".format(
##        version_time, target, n_bits)]
##                                     
##    env["CCFLAGS"] = my_ccflags
##    env["LINKFLAGS"] = my_ccflags
##
##    if target == "win":
##        env["CC"] = "mingw32-gcc"
##        env["LINK"] = "mingw32-gcc"
##
##    build_dir = os.path.join("build",
##                             "{0}{1}".format(target, n_bits),
##                             status)
##    SConscript(os.path.join("src", "SConscript"),
##               variant_dir=build_dir,
##               exports={"env": my_env})
