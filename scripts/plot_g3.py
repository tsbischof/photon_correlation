#!/usr/bin/env python

import sys

import matplotlib.pyplot as plt

from photon_correlation import G3_T3

if __name__ == "__main__":
    for filename in sys.argv[1:]:
        g3 = G3_T3(filename=filename)

        fig = g3.make_figure()

        plt.show(fig)
        plt.close(fig)
