#!/usr/bin/env python

import sys

total = 0
for line in sys.stdin:
    total += int(line.strip().split(",")[-1])

print(total)
