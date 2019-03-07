# http://trac.secdev.org/scapy/ticket/31 

# scapy.contrib.description = MPLS
# scapy.contrib.status = loads

from scapy.packet import Packet, bind_layers, Padding
from scapy.fields import BitField, ByteField, ShortField
from scapy.layers.inet import IP, UDP
from scapy.layers.inet6 import IPv6
from scapy.layers.l2 import Ether

class MPLS(Packet): 
    name = "MPLS" 
    fields_desc =  [ BitField("label", 3, 20), 
                    BitField("cos", 0, 3), 
                    BitField("s", 1, 1), 
                    ByteField("ttl", 0)  ] 

    def guess_payload_class(self, payload):
        if len(payload) >= 1:
            if not self.s:
                return MPLS
            ip_version = payload[0] >> 4
            if ip_version == 4:
                return IP
            elif ip_version == 6:
                return IPv6
            else:
                return Padding
        return Padding


bind_layers(Ether, MPLS, type=0x8847)
bind_layers(IP, MPLS, proto=137)
bind_layers(IPv6, MPLS, nh=137)
bind_layers(UDP, MPLS, dport=6635)
bind_layers(MPLS, MPLS, s=0)
