# photon correlation
Command-line tools for processing photon arrival time (TCSPC / TTTR) data

## Building
This code is written in C with no external dependencies. To build:
```
./bootstrap
./configure
make
```

## Overview
This project generates a suite of command-line tools designed for processing photon arrival time data. 
They are designed for minimum memory usage and maximum flexibility, rather than for absolute performance.

Typical workflows include:
* calculating a g2 or nth-order correlation
* histogramming events for lifetime analysis
* measuring intensity over time
* all of the above, simulatenously

These programs expect photon arrival time data in the same format as [libpicoquant](https://github.com/tsbischof/libpicoquant). 
See [below](#data-formats) for more details.

## Quick guide for usage
### Lifetimes (t3 data only)
```
photon_gn --file-in t3.csv --mode t3 --order 1 --time 0,1024,524288 --file-out t3.csv.g1
```

### g2
`photon_gn` will generate the correlation (g2) and intensity trace simulaneously.
For t2 we specify the time bins:
```
photon_gn --file-in t2.csv --mode t2 --order 2 --time -524544,2048,524544 --file-out t2.csv.g2
```
For t3 we need to specify both time and pulse:
```
photon_gn --file-in t3.csv --mode t3 --order 2 --time -524544,1,524544 --pulse -10.5,21,10.5 --file-out t3.csv.g2
```

## Programs
Each program gives far more detail with `--help`.
There is a detailed description for some of the programs in `doc/`, and in [this PhD thesis](https://dspace.mit.edu/handle/1721.1/101453)

### photon_gn
This covers most use cases.
It is designed to take in TTTR data and output intensity traces, correlations, and other useful analyzable data.

### photon_correlate
Takes in photons and outputs correlated tuples

### photon_histogram
Creates histograms of photons (t3 g1) or correlations (g2 and gn)

### photon_intensity
Measures the number of photons which arrived during a given interval

### intensity_correlate, photon_intensity_correlate
Performs multi-tau correlation for fixed-interval intensity data (intensity_correlate), or for TTTR data (photon_intensity_correlate)

### photon_intensity_dependent_gn, photon_flid
Acts like photon_gn, but with a different calculated gn based on the instantaneous intensity.
`photon_flid` is specific to the lifetime case.

### photon_number, photon_number_to_channels
For t3 data only.
`photon_number` reports the number of times there were 0, 1, 2, ... photons arriving after a given sync pulse. 
`photon_number_to_channels` instead modifies the channel number to indicate how many photons arrived in that pulse, and in what order.

### photons
Converts to and from t2 and t3 data. 

### photon_synced_t2
Treats a channel of t2 data as the sync signal.

### photon_t3_offsets
Shifts the arrival time and pulse of t3 data, used if the relative timing of the sync and photon detector were incorrect.

### photon_temper
Modifies the photon_stream to do things like:
* add time offset
* drop channels
* keep only the first photon arriving after a given pulse (e.g. to suppress afterpulsing)
* apply time gating (only keep photons which arrived some time after the sync)

## Data formats
All data formats are headerless csv, in one of the following types.
See `sample_data/` for examples.

### Photons (input and output)
#### T2
In ascii format:
```
channel (uint32), arrival time (uint64)
```

#### T3
In ascii format: 
```
channel (uint32), arrival pulse (uint64), arrival time (uint64)
```

### Correlations (input and output)
Correlations are pairs (triplets, quadruplets, ... generally tuples) of photons, with some difference in time (or pulse number). 

### Histogram bins (output)
#### T2 histogram
For a g2:
```
channel photon 0 (uint32), channel photon 1 (uint32), time bin left (uint64), time bin right (uint64), counts (uint64)
```
where time bins are closed-open ranges (`[left, right)`).

For a g3: 
```
channel photon 0 (uint32), channel photon 1 (uint32), time bin 0 left (uint64), time bin 0 right (uint64), time bin 1 left (uint64), time bin 1 right (uint64), counts (uint64)
```
This idea extends to higher orders as well, just with more time bin definitions.

#### T3 histogram
For a g2:
```
channel photon 0 (uint32), channel photon 1 (uint32), pulse bin 0 left (uint64), pulse bin 0 right (uint64), time bin 0 left (uint64), time bin 0 right (uint64), counts
```
As with the [t2 data](#t2-histogram) this idea can be extended to higher orders.
