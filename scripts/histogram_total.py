#!/usr/bin/env python

import csv
import sys

total = 0
for channel, pulse_number, counts in csv.reader(sys.stdin):
    total += int(counts)

print(total)
