#!/usr/bin/env python

import subprocess
import os

for program in map(lambda x: x[:-4],
                   filter(lambda y: y.endswith(".tex"),
                          os.listdir(os.getcwd()))):
    try:
        usage = subprocess.Popen([program,
                                  "--help"],
                                 stderr=subprocess.PIPE)
        with open("{0}.usage".format(program), "w") as usage_file:
            usage_file.write(r"\begin{verbatim}")
            for line in usage.stderr:
                usage_file.write(line.decode())
            usage_file.write(r"\end{verbatim}")
    except:
        print("Could not process {0}".format(program))
