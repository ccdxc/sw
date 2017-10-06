#! /usr/bin/python3
import pdb

# Support options
# start         : Starting position in the packet.
# end           : Ending position in the packet
# startoffset   : Offset from the 'start'
# endoffset     : Offset from the 'end'
# starthdr      : Starting Header ID (Derives 'start')
# endhdr        : Ending Header ID (Derives 'end')

class PacketSlicer:
    def __init__(self, packet, args):
        self.packet     = packet
        self.args       = args
        self.start      = getattr(args, 'start', None)
        self.end        = getattr(args, 'end', None)
        self.sof        = getattr(args, 'startoffset', 0)
        self.eof        = getattr(args, 'endoffset', 0)
        self.starthdr   = getattr(args, 'starthdr', None)
        self.endhdr     = getattr(args, 'endhdr', None)

        self.__validate_args()
        return

    def __get_hdr_start(self, hdrid):
        hdr = getattr(self.packet.headers, hdrid, None)
        if hdr is None:
            return None
        return hdr.start

    def __get_hdr_size(self, hdrid):
        hdr = getattr(self.packet.headers, hdrid, None)
        if hdr is None:
            assert(0)
        return hdr.size

    def __get_start_by_hdr(self):
        if self.starthdr is None:
            return None
        return self.__get_hdr_start(self.starthdr)

    def __get_end_by_hdr(self):
        if self.endhdr is None:
            return None
        end = self.__get_hdr_start(self.endhdr) + self.__get_hdr_size(self.endhdr)
        return end

    def __validate_args(self):
        if self.start is not None and self.starthdr is not None:
            assert(0)
        if self.end is not None and self.endhdr is not None:
            assert(0)
        return

    def __get_start(self):
        if self.start is not None:
            return self.start + self.sof
        start = self.__get_start_by_hdr()
        if start is None:
            return self.sof
        return start + self.sof

    def __get_end(self):
        if self.end is not None:
            return self.end + self.eof
        end = self.__get_end_by_hdr()
        if end is None:
            return None
        return end + self.eof

    def GetSlice(self):
        self.sidx = self.__get_start()
        self.eidx = self.__get_end()
        return self.packet.rawbytes[self.sidx:self.eidx]

def GetPacketSlice(tc, packet, args):
    srcpacket = tc.packets.Get(args.pktid) 
    slicer = PacketSlicer(srcpacket, args)
    slc = slicer.GetSlice()
    return list(slc)
