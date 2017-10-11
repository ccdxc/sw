

from infra.common.logging       import cfglogger
from scapy.all import *


def GetRxCqPacketFlags(tc, obj, args=None):
    pkt = tc.packets.db['PKT2'].spktobj.spkt
    fcs_ok = 1
    ipv4_valid = 1 if pkt.haslayer(IP) else 0
    ipv6_valid = 1 if pkt.haslayer(IPv6) else 0
    vlan_valid = 1 if tc.config.dst.endpoint.intf.lif.vlan_strip_en else 0
    frag = 0
    tunneled = 0
    flags = ((tunneled << 5) | (frag << 4) |
            (ipv6_valid << 3) | (ipv4_valid << 2) |
            (vlan_valid << 1) | fcs_ok)
    cfglogger.info(">>>> FLAGS (%s) :"
                   " fcs_ok=%d vlan_valid=%d"
                   " ipv4_valid=%d ipv6_valid=%d"
                   " frag=%d tunneled=%d <<<<" % (
                    flags,
                    fcs_ok, vlan_valid,
                    ipv4_valid, ipv6_valid,
                    frag, tunneled))
    return flags
