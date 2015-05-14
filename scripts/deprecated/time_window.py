#!/usr/bin/env python3

import csv
import argparse
import datetime

class Limits(object):
    def __init__(self, lower, upper):
        self.lower = lower
        self.upper = upper

    def __str__(self):
        return("{},{}".format(self.lower, self.upper))

    def contains(self, pulse):
        return(self.lower <= pulse and pulse <= self.upper)

def trim_t3_g2(filename, pulse_limits, args):
    with open(filename) as stream_in:
        with open("{0}.{1:.1f}_{2:.1f}".format(filename,
                                               pulse_limits.lower,
                                               pulse_limits.upper),
                  "w") as stream_out:
            writer = csv.writer(stream_out)
            
            for index, line in enumerate(csv.reader(stream_in)):
                if args.print_every and index % args.print_every == 0:
                    print("{}: Line {}".format(
                        datetime.datetime.today().replace(
                            microsecond=0).isoformat(),
                        index))
                    
                pulse_lower, pulse_upper = map(float, line[2:4])

                if pulse_limits.contains(pulse_lower) or \
                   pulse_limits.contains(pulse_upper):
                    writer.writerow(line)
    
if __name__ == "__main__":
##    test = ["--pulse", "-100000", "100000", "--print-every", "1000000",
##            "../sample_data/030414_IC_S2_longwindow_highres.g2.run/g2"]
##    
    parser = argparse.ArgumentParser(
        description="Trim down a gn file to a given pulse window",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("--pulse", type=float, nargs=2, required=True,
                        help="Pulse limits, listed as 'start stop', with a "
                        "space. Any pulse bin containing these values or a "
                        "value between them will be retained.")
    parser.add_argument("--order", type=int, default=2,
                        help="The order of the correlation represented.")
    parser.add_argument("--mode", type=str, default="t3",
                        help="The mode of the correlation represented.")
    parser.add_argument("files", type=str, nargs="*",
                        help="Filenames to process.")
    parser.add_argument("--print-every", type=int, default=0,
                        help="Print a status message every n lines processed.")

    args = parser.parse_args()

    pulse_limits = Limits(args.pulse[0], args.pulse[1])
    
    for filename in args.files:
        if args.mode == "t3":
            if args.order == 2:
                trim_t3_g2(filename, pulse_limits, args)
            else:
                raise(ValueError("Unsupported order: {}".format(args.order)))
        else:
            raise(ValueError("Unsupported mode: t2"))
    
    
