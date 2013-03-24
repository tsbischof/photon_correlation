import csv
import operator

class PhotonNumber(object):
    def __init__(self, filename):
        self._numbers = dict()

        self.from_file(filename)

    def __str__(self):
        return("({0})".format(", ".join(map(str,  sorted(self)))))

    def __setitem__(self, index, value):
        self._numbers[index] = value

    def __getitem__(self, index):
        if index not in self._numbers.keys():
            return(0)
        else:
            return(self._numbers[index])

    def __iter__(self):
        for n in sorted(self._numbers.keys()):
            yield((n, self[n]))

    def number(self):
        return(tuple(map(operator.itemgetter(0), sorted(self))))

    def seen(self):
        return(tuple(map(operator.itemgetter(1), sorted(self))))

    def from_file(self, filename):
        with open(filename) as stream_in:
            for number, seen in csv.reader(stream_in):
                number = int(number)
                seen = int(seen)

                self[number] = seen

    def plot(self):
        import matplotlib.pyplot as plt

        plt.clf()
        plt.semilogy(self.number(), self.seen())
        plt.show()

    def fractions(self):
        total = float(sum(self.seen()))

        for seen in self.seen():
            yield(seen/total)
    
if __name__ == "__main__":
    numbers = PhotonNumber("../src/blargh.number")
    for n, val in numbers:
        print(n, val)

    print(str(numbers))
##    numbers.plot()
    print(tuple(numbers.fractions()))
