import os
import subprocess
import itertools

from test import diff

class CorrelateTester(object):
    def __init__(self, old_exe, new_exe):
        self.old_exe = old_exe
        self.new_exe = new_exe

        if not os.path.isfile(self.old_exe):
            raise(NameError(self.old_exe, "cannot be found."))

        if not os.path.isfile(self.new_exe):
            raise(NameError(self.new_exe, "cannot be found."))

    def test(self, data_filename,
             mode, order,
             min_time_distance, max_time_distance,
             min_pulse_distance, max_pulse_distance,
             positive_only, start_stop):
        cmd = ["--file-in", data_filename]
        
        if start_stop:
            cmd.extend(["--start-stop", "--mode", "t2"])

            if mode != "t2":
                raise(ValueError("Start-stop mode requires t2 photons"))
        else:           
            cmd.extend(["--mode", mode, "--order", str(order)])

            if not min_time_distance is None:
                cmd.extend(["--min-time-distance", str(min_time_distance)])

            if not max_time_distance is None:
                cmd.extend(["--max-time-distance", str(max_time_distance)])

            if not min_pulse_distance is None:
                cmd.extend(["--min-pulse-distance", str(min_pulse_distance)])

            if not max_pulse_distance is None:
                cmd.extend(["--max-pulse-distance", str(max_pulse_distance)])

            if positive_only:
                cmd.append("--positive-only")

        old_cmd = [self.old_exe] + cmd
        new_cmd = [self.new_exe] + cmd
        
        old = subprocess.Popen(old_cmd, stdout=subprocess.PIPE)
        new = subprocess.Popen(new_cmd, stdout=subprocess.PIPE)

        return(diff(old.stdout, new.stdout), cmd)

if __name__ == "__main__":
    old = "../bin/known_good/correlate"
    new = "../bin/testing/correlate"
    tester = CorrelateTester(old, new)

    parameters = list()

    for data_filename, order, min_time_distance, \
        max_time_distance in itertools.product(\
            ["../sample_data/t2.txt"],
            range(2, 8),
            [None, 1, 2],
            [None, 5, 10]):
        parameters.append((data_filename,
                           "t2", order,
                           min_time_distance, max_time_distance,
                           None, None,
                           False, False))

    for data_filename, order, min_time_distance, \
        max_time_distance in itertools.product(\
            ["../sample_data/t2.txt"],
            range(2, 8),
            [None, 1, 2],
            [None, 5, 10]):
        parameters.append((data_filename,
                           "t2", order,
                           min_time_distance, max_time_distance,
                           None, None,
                           False, False))


    for parameter in parameters:
        print(parameter)
        result, cmd = tester.test(*parameter)

        if not result:
            print("Failure: ", cmd)
