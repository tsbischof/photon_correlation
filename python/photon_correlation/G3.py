import csv

from GN import GN

from util import is_cross_correlation

class G3_T3(GN):
    def from_stream(self, stream_in):
        for c0, c1, p10, p11, t10, t11, c2, p20, p21, t20, t21, counts \
            in stream_in:
            correlation = tuple(map(int, (c0, c1, c2)))
            
            p1 = tuple(map(float, (p10, p11)))
            t1 = tuple(map(float, (t10, t11)))
            p2 = tuple(map(float, (p20, p21)))
            t2 = tuple(map(float, (t20, t21)))
            
            counts = int(counts)

            if correlation not in self._counts:
                self[correlation] = dict()

            if p1 not in self._counts[correlation]:
                self._counts[correlation][p1] = dict()

            if t1 not in self._counts[correlation][p1]:
                self._counts[correlation][p1][t1] = dict()

            if p2 not in self._counts[correlation][p1][t1]:
                self._counts[correlation][p1][t1][p2] = dict()

            if t2 not in self._counts[correlation][p1][t1][p2]:
                self._counts[correlation][p1][t1][p2][t2] = dict()

            self._counts[correlation][p1][t1][p2][t2] = counts

    def to_stream(self):
        for correlation, gn in self:
            for p1 in sorted(gn):
                for t1 in sorted(gn[p1]):
                    for p2 in sorted(gn[p1][t1]):
                        for t2 in sorted(gn[p1][t1][p1]):
                            line = list(correlation[:2]) + \
                                   list(p1) + list(t1) + \
                                   [correlation[2]] + \
                                   list(p2) + list(t2) + \
                                   [gn[p1][t1][p2][t2]]

                            yield(line)

    def unique_peaks(self):
        """
        Report the center, diagonal, and off-diagonal values.
        """
        peaks = {"center": 0,
                 "diagonal": 0,
                 "off-diagonal": 0}

        for correlation, gn in self:
            print(correlation, is_cross_correlation(correlation))
            if is_cross_correlation(correlation):
                for p1, p2, peak in [((-0.5, 0.5), (-0.5, 0.5), "center"),
                                     ((-0.5, 0.5), (0.5, 1.5), "diagonal"),
                                     ((0.5, 1.5), (1.5, 2.5), "off-diagonal")]:
                    for t1 in gn[p1]:
                        print(correlation)
                        print(t1)
                        print(gn[p1][t1][p2].items())
                        peaks[peak] += sum(gn[p1][t1][p2].values())
            
        return(peaks)

class G3_T2(GN):
    pass

if __name__ == "__main__":
##    g3 = G3_T3(filename="/home/tsbischof/Documents/data/"
##                "microscopy/analysis/triexciton/"
##                "2014-09-04_oc2014-04-08/"
##                "oc2014-04-08_1e-5_dot_009_250nW_000.ht3.idgn.run/"
##                "g3.5000.threshold.0.70")

    g3 = G3_T3(filename="/home/tsbischof/Documents/data/"
                "microscopy/analysis/triexciton/"
                "2014-09-04_oc2014-04-08/"
                "oc2014-04-08_1e-5_dot_009_250nW_000.ht3.g3.run/"
                "g3")

    g3.to_file("/home/tsbischof/tmp/blargh.g3")

##    print(g3.unique_peaks())
