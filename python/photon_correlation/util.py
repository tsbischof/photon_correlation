import re
import os
import sys
import functools
import itertools
import collections
import statistics

import matplotlib as mpl
import matplotlib.cm as cm
import numpy

def factorial(n):
    return(functools.reduce(lambda x, y: x*y, range(1, n+1), 1.0))

def get_colors(n, lower=None, upper=None, colormap=cm.rainbow):
    if lower is None:
        lower = 0
        
    if upper is None:
        upper = 1
        
    return(colormap(numpy.linspace(lower, upper, n)))

def dot_number(filename):
    raw = filename_parser.search(filename).group("dot_number")
    parsed = dot_number_parser.search(raw)

    return(int(parsed.group("index")), parsed.group("modifier"))

def normalize(L, key=max):
    my_key = key(L)

    if my_key == 0:
        my_key = 1

    return(list(map(lambda x: x/float(my_key), L)))

def first_index(L, value):
    """
    Find the first occurrence of value in L.
    """
    val = next(iter(filter(lambda x: x[1] == value, enumerate(L))))

    if val:
        return(val[0])
    else:
        raise(ValueError("{} is not in the list.".format(value)))

def last_index(L, value):
    """
    Find the final occurrence of value in L.
    """
    val = next(iter(
        filter(lambda x: x[1] == value, reversed(list(enumerate(L))))))

    if val:
        return(val[0])
    else:
        raise(ValueError("{} is not in the list.".format(value)))

def is_cross_correlation(correlation):
    return(len(set(correlation)) == len(correlation))

def common_name(filename):
    my_filename = filename
    root = filename
    found_data = False
    while not found_data:
        if not root:
            raise(ValueError("Filename does not appear to "
                             "correspond to a dot: {}".format(filename)))
        
        root, my_filename = os.path.split(my_filename)
        
        if filename_parser.search(my_filename):
            found_data = True
        else:
            my_filename = root
             
    root, date = os.path.split(root)

    return("{}_{}".format(date, re.sub("\.ht[23]*", "", my_filename)))

def rebin(L, n=2):
    """
    Group every n elements, and add them together. Division by n creates
    the boxcar average.
    """
    result = list()
    
    for i in range(len(L)//n):
        result.append(sum(L[(i*n):((i+1)*n)]))

    return(result)

def smooth(L, n=2):
    """
    Perform a boxcar average with width n. Return the result as a new
    lifetime object.
    """
    return(list(map(lambda x: float(x)/n, rebin(list(L), n=n))))

def final_nonzero(L):
    """
    Return the index of the last non-zero value in the list.
    """
    for index, val in reversed(list(enumerate(L))):
        if val:
            return(index)

    return(0)

def transpose(L):
    length = len(L[0])

    for i in range(length):
        yield(list(map(lambda x: x[i], L)))

def ps_to_ns(L):
    return(list(map(lambda x: x*1e-3, L)))

def make_gn_stream(bins, counts):
    """
    Given the bins (header) and counts (row) of a single line of a gn.td run,
    rotate the result to produce the stream appropriate for the gn file format.
    """
    for my_bin, my_counts in zip(numpy.transpose(bins),
                                 numpy.transpose(counts)):
        yield(list(my_bin) + [my_counts])

def neighbor_normalize(times, counts):
    """
    Use the distance between nearest neighbors to add a correction to the
    normalization of the counts.
    """
    new_times = list()
    new_counts = list()

    # Start away from the ends to make sure that we do not try to look outside
    # the list
    for index, vals in enumerate(zip(times, counts[1:-2])):
        t, c = vals
        
        if c != 0:
            left = None
            right = None

            for left_index in reversed(range(index)):
                if counts[left_index] != 0:
                    left = left_index
                    break

            for right_index in range(index+1, len(counts)):
                if counts[right_index] != 0:
                    right = right_index
                    break
  
            if left is not None and right is not None:
                dt = statistics.mean(times[right]) - \
                     statistics.mean(times[left])
                new_left = statistics.mean([statistics.mean(times[left]),
                                            statistics.mean(t)])
                new_right = statistics.mean([statistics.mean(t),
                                             statistics.mean(times[right])])
                new_times.append((new_left, new_right))
                new_counts.append(c/(dt*2.0))

    return(new_times, new_counts)

def unique_dots(dots):
    """
    Group distinct runs for single dots to enable averaging over all data.
    """
    keys = set(map(lambda x: x.dot_key(), dots))

    for key in sorted(keys):
        yield(list(filter(lambda x: x.dot_key() == key, dots)))

def unique_dot_keys(experiment_keys):
    """
    Return the first run of each dot, given all experiments.
    """
    result = list()
        
    for key in sorted(experiment_keys):
        if key[:2] not in map(lambda x: x[:2], result):
            result.append(key)

    return(result)

def dot_index(dot, dots_dict):
    """
    Determine the absolute index of the dot, given the dict of dots. Index
    starts at 1.
    """
    keys = list(sorted(set(map(lambda x: x[:2], dots_dict.keys()))))
    return(keys.index(dot.dot_key())+1)

def flatten(LoL):
    return(list(itertools.chain.from_iterable(LoL)))
