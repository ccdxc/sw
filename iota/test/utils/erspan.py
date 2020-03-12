# Encapsulated Remote Switch Packet Analysis

from scapy.packet import *
from scapy.fields import *
from scapy.all import *

class ERSPAN(Packet):
    name = "ERSPAN"
    fields_desc = [ BitField("version", 1, 4),
                    BitField("vlan", 0, 12),
                    BitField("priority", 0, 3),
                    BitField("unknown2", 0, 1),
                    BitField("direction", 0, 1),
                    BitField("truncated", 0, 1),
                    BitField("span_id", 0, 10),
                    XIntField("unknown7", 0x00000000)]

bind_layers(GRE, ERSPAN, proto=0x88be)
bind_layers(ERSPAN, Ether)

class ERSPAN_III(Packet):
    name = "ERSPAN_III"
    fields_desc = [ BitField("version", 2, 4),
                    BitField("vlan", 0, 12),
                    BitField("cos", 0, 3),
                    BitField("bos", 0, 2),
                    BitField("truncated", 0, 1),
                    BitField("span_id", 0, 10),
                    XIntField("timestamp", 0x00000000),
                    BitField("sgt", 0, 16),
                    BitField("pdu", 0, 1),
                    BitField("frame_type", 0, 5),
                    BitField("hw_id", 0, 6),
                    BitField("direction", 0, 1),
                    BitField("granularity", 0, 2),
                    BitField("options", 0, 1)]

class PlatformSpecific(Packet):
    name = "PlatformSpecific"
    fields_desc = [ BitField("platf_id", 0, 6),
                    BitField("reserved", 0, 12),
                    BitField("port_id", 0, 14),
                    XIntField("timestamp", 0x00000000)]

bind_layers(GRE, ERSPAN_III, proto=0x22eb)
bind_layers(ERSPAN_III, Ether, options=0)
bind_layers(ERSPAN_III, PlatformSpecific, options=1)
bind_layers(PlatformSpecific, Ether)
