from ctypes import *
import datetime

from photon_correlation.picoquant import picoquant
from photon_correlation import modes

class ph_v20_display_curve_t(Structure):
    _fields_ = [("MapTo", c_int32),
                ("Show", c_int32)]

class ph_v20_param_t(Structure):
    _fields_ = [("Start", c_float),
                ("Step", c_float),
                ("Stop", c_float)]
    
class ph_v20_router_channel_t(Structure):
    _fields_ = [("InputType", c_int32),
                ("InputLevel", c_int32),
                ("InputEdge", c_int32),
                ("CFDPresent", c_int32),
                ("CFDLevel", c_int32),
                ("CFDZCross", c_int32)]

class ph_v20_board_t(Structure):
    _fields_ = [("HardwareIdent", c_char*16),
                ("HardwareVersion", c_char*8),
                ("HardwareSerial", c_int32),
                ("SyncDivider", c_int32),
                ("CFDZeroCross0", c_int32),
                ("CFDLevel0", c_int32),
                ("CFDZeroCross1", c_int32),
                ("CFDLevel1", c_int32),
                ("Resolution", c_float),
                ("RouterModelCode", c_int32),
                ("RouterEnabled", c_int32),
                ("RtCh", POINTER(ph_v20_router_channel_t))]

class ph_v20_header_t(Structure):
    _fields_ = [("CreatorName", c_char*18),
                ("CreatorVersion", c_char*12),
                ("FileTime", c_char*18),
                ("CRLF", c_char*2),
                ("Comment", c_char*256),
                ("NumberOfCurves", c_int32),
                ("BitsPerRecord", c_int32),
                ("RoutingChannels", c_int32),
                ("NumberOfBoards", c_int32),
                ("ActiveCurve", c_int32),
                ("MeasurementMode", c_int32),
                ("SubMode", c_int32),
                ("RangeNo", c_int32),
                ("Offset", c_int32),
                ("AcquisitionTime", c_int32),
                ("StopAt", c_int32),
                ("StopOnOvfl", c_int32),
                ("Restart", c_int32),
                ("DisplayLinLog", c_int32),
                ("DisplayTimeAxisFrom", c_int32),
                ("DisplayTimeAxisTo", c_int32),
                ("DisplayCountAxisFrom", c_int32),
                ("DisplayCountAxisTo", c_int32),
                ("DisplayCurve", ph_v20_display_curve_t*8),
                ("Param", ph_v20_param_t*3),
                ("RepeatMode", c_int32),
                ("RepeatsPerCurve", c_int32),
                ("RepeatTime", c_int32),
                ("RepeatWaitTime", c_int32),
                ("ScriptName", c_char*20),
                ("Brd", POINTER(ph_v20_board_t))]

    def __init__(self):
        super(ph_v20_header_t, self).__init__()

    def set_time(self, current_time=datetime.datetime.now()):
        self.FileTime = current_time.strftime("%y.%m.%d %H.%M.%S").encode()

class ph_v20_tttr_header_t(Structure):
    _fields_ = [("ExtDevices", c_int32),
                ("Reserved", c_int32*2),
                ("InpRate0", c_int32),
                ("InpRate1", c_int32),
                ("StopAfter", c_int32),
                ("StopReason", c_int32),
                ("NumRecords", c_int32),
                ("ImgHdrSize", c_int32),
                ("ImgHdr", POINTER(c_uint32))]

class ph_v20_header:
    def __init__(self):
        self.pq_header = picoquant.pq_header_t()
        self.pq_header.Ident = "PicoHarp 300".encode()
        self.pq_header.FormatVersion = "2.0".encode()

        self.ph_header = ph_v20_header_t()
        self.ph_header.CreatorName = "Bischof microscopy".encode()
        self.ph_header.set_time()
        self.ph_header.CRLF = "\r\n".encode()
        self.ph_header.Comment = "T2 data from Thomas Bischof's" \
                                 "microscope.".encode()
        self.ph_header.NumberOfCurves = 0
        self.ph_header.BitsPerRecord = 32
        self.ph_header.RoutingChannels = 4
        self.ph_header.NumberOfBoards = 1
        self.ph_header.ActiveCurve = 0
        self.ph_header.MeasurementMode = modes.MODE_T2
        self.ph_header.SubMode = 0
        self.ph_header.RangeNo = 0
        self.ph_header.Offset = 0
        self.ph_header.AcquisitionTime = 0
        self.ph_header.StopAt = 65535
        self.ph_header.StopOnOvfl = 0
        self.ph_header.Restart = 0
        self.ph_header.DisplayLinLog = 1
        self.ph_header.DisplayTimeAxisFrom = 0
        self.ph_header.DisplayTimeAxisTo = 400
        self.ph_header.DisplayCountAxisFrom = 0
        self.ph_header.DisplayCountAxisTo = 1000000

        for i in range(8):
            self.ph_header.DisplayCurve[i].MapTo = i
            self.ph_header.DisplayCurve[i].Show = 1

        for i in range(3):
            self.ph_header.Param[i].Start = 0.0
            self.ph_header.Param[i].Step = 0.0
            self.ph_header.Param[i].Stop = 0.0

        self.ph_header.RepeatMode = 0
        self.ph_header.RepeatsPerCurve = 1
        self.ph_header.RepeatTime = 0
        self.ph_header.RepeatWaitTime = 0
        self.ph_header.ScriptName = "blargh!".encode()

        self._RtCh = list()
        self.ph_header.Brd = (ph_v20_board_t*self.ph_header.NumberOfBoards)()
        for i in range(self.ph_header.NumberOfBoards):
            self.ph_header.Brd[i].HardwareIdent = "PicoHarp 300".encode()
            self.ph_header.Brd[i].HardwareVersion = "2.0".encode()
            self.ph_header.Brd[i].HardwareSerial = 0xBEEF
            self.ph_header.Brd[i].SyncDivider = 1
            self.ph_header.Brd[i].CFDZeroCross0 = 0
            self.ph_header.Brd[i].CFDLevel0 = 10
            self.ph_header.Brd[i].CFDZeroCross1 = 10
            self.ph_header.Brd[i].CFDLevel1 = 100
            self.ph_header.Brd[i].Resolution = 0.128
            self.ph_header.Brd[i].RouterModelCode = 0
            self.ph_header.Brd[i].RouterEnabled = 1

            self._RtCh.append(
                (ph_v20_router_channel_t*
                 self.ph_header.RoutingChannels)())
            self.ph_header.Brd[i].RtCh = self._RtCh[i]

            for j in range(self.ph_header.RoutingChannels):
                self.ph_header.Brd[i].RtCh[j].InputType = 0
                self.ph_header.Brd[i].RtCh[j].InputLevel = 0
                self.ph_header.Brd[i].RtCh[j].InputEdge = 0
                self.ph_header.Brd[i].RtCh[j].CFDPresent = 0
                self.ph_header.Brd[i].RtCh[j].CFDLevel = 0
                self.ph_header.Brd[i].RtCh[j].CFDZCross = 0

        self.tttr_header = ph_v20_tttr_header_t()
        self.tttr_header.ExtDevices = 0;
        self.tttr_header.Reserved[0] = 0xDEAD
        self.tttr_header.Reserved[1] = 0xBEEF
        self.tttr_header.InpRate0 = 0
        self.tttr_header.InpRate1 = 0
        self.tttr_header.StopAfter = 0
        self.tttr_header.StopReason = 0
        self.tttr_header.NumRecords = 0
        self.tttr_header.ImgHdrSize = 0

if __name__ == "__main__":
    header = ph_v20_header()
    print(header.pq_header.Ident)
    print(header.ph_header.Brd[0].RtCh[1].InputType)
    
