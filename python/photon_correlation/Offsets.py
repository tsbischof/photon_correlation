class Offsets(object):
    def __init__(self, *offsets):
        self.offsets = list(offsets)

    def __str__(self):
        return(",".join(map(str, self.offsets)))

    def __getitem__(self, index):
        return(self.offsets[index])

    def __setitem__(self, index, offset):
        self.offsets[index] = offset

    def __len__(self):
        return(len(self.offsets))

    def all_same(self):
        return(len(set(self.offsets)) == 1)

