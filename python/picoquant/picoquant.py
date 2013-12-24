import ctypes

class pq_header_t(ctypes.Structure):
    _fields_ = [("Ident", ctypes.c_char*16),
                ("FormatVersion", ctypes.c_char*6)]

if __name__ == "__main__":
    pq_header = pq_header_t()
    pq_header.Ident = "blargh".encode()
