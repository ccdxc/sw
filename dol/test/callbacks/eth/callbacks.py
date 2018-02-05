
import struct
from socket import inet_aton
from scapy.layers.inet import ICMP
from scapy.utils import checksum
import config.hal.defs as haldefs
from bitstring import BitArray

from .toeplitz import *


LifRssType = haldefs.interface.LifRssType


RssType2Enum = {
    LifRssType.Value("RSS_TYPE_NONE"): RSS.NONE,
    LifRssType.Value("RSS_TYPE_IPV4"): RSS.IPV4,
    LifRssType.Value("RSS_TYPE_IPV4") + LifRssType.Value("RSS_TYPE_IPV4_TCP"): RSS.IPV4_TCP,
    LifRssType.Value("RSS_TYPE_IPV4") + LifRssType.Value("RSS_TYPE_IPV4_UDP"): RSS.IPV4_UDP,
    LifRssType.Value("RSS_TYPE_IPV6"): RSS.IPV6,
    LifRssType.Value("RSS_TYPE_IPV6") + LifRssType.Value("RSS_TYPE_IPV6_TCP"): RSS.IPV6_TCP,
    LifRssType.Value("RSS_TYPE_IPV6") + LifRssType.Value("RSS_TYPE_IPV6_UDP"): RSS.IPV6_UDP,
}


def GetRxRssTypeNum(tc, obj, args=None):
    pkt = tc.packets.db['PKT2'].spktobj.spkt
    lif = tc.config.dst.endpoint.intf.lif
    flags = []
    if pkt.haslayer(IP):
        flags.append(LifRssType.Value("RSS_TYPE_IPV4"))
        if pkt.haslayer(TCP):
            flags.append(LifRssType.Value("RSS_TYPE_IPV4_TCP"))
        elif pkt.haslayer(UDP):
            flags.append(LifRssType.Value("RSS_TYPE_IPV4_UDP"))
    if pkt.haslayer(IPv6):
        flags.append(LifRssType.Value("RSS_TYPE_IPV6"))
        if pkt.haslayer(TCP):
            flags.append(LifRssType.Value("RSS_TYPE_IPV6_TCP"))
        elif pkt.haslayer(UDP):
            flags.append(LifRssType.Value("RSS_TYPE_IPV6_UDP"))

    return RssType2Enum[lif.rss_type & sum(flags)]


def GetRxRssType(tc, obj, args=None):
    rss_type_num = GetRxRssTypeNum(tc, obj, args)
    return rss_type_num.value


def GetRxRssHash(tc, obj, args=None):
    pkt = tc.packets.db['PKT2'].spktobj.spkt
    lif = tc.config.dst.endpoint.intf.lif
    rss_type_num = GetRxRssTypeNum(tc, obj, args)

    # Pass the RSS key as args to test
    return toeplitz_hash(toeplitz_input(rss_type_num, pkt), BitArray(bytes=toeplitz_msft_key))


def GetChecksumTestPacket(tc, obj, args=None):
    spktobj = tc.packets.db['PKT1'].spktobj
    pkt = spktobj.spkt
    if pkt.haslayer(IP):
        if pkt.haslayer(TCP):
            psdhdr = struct.pack("!4s4sHH",
                                 inet_aton(pkt[IP].src),
                                 inet_aton(pkt[IP].dst),
                                 pkt[IP].proto,
                                 len(pkt[TCP]))
            pkt[TCP].chksum = ~checksum(psdhdr) & 0xffff
        elif pkt.haslayer(UDP):
            psdhdr = struct.pack("!4s4sHH",
                                 inet_aton(pkt[IP].src),
                                 inet_aton(pkt[IP].dst),
                                 pkt[IP].proto,
                                 len(pkt[UDP]))
            pkt[UDP].chksum = ~checksum(psdhdr) & 0xffff
        elif pkt.haslayer(ICMP):
            # https://tools.ietf.org/html/rfc792
            pkt[ICMP].chksum = 0
        else:
            raise NotImplementedError
    elif pkt.haslayer(IPv6):
        from scapy.layers.inet6 import in6_chksum
        if pkt.haslayer(TCP):
            pkt[TCP].chksum = ~in6_chksum(pkt[IPv6].nh, pkt[TCP], bytes([0x0]*len(pkt[TCP]))) & 0xffff
        elif pkt.haslayer(UDP):
            pkt[UDP].chksum = ~in6_chksum(pkt[IPv6].nh, pkt[UDP], bytes([0x0]*len(pkt[UDP]))) & 0xffff
        elif pkt[IPv6].nh == 58: # ICMP
            # https://tools.ietf.org/html/rfc4443#section-2.3
            pkt[IPv6].payload.cksum = ~in6_chksum(pkt[IPv6].nh, pkt[IPv6].payload,
                                                  bytes([0x0]*len(pkt[IPv6].payload))) & 0xffff
        else:
            raise NotImplementedError
    else:
        raise NotImplementedError
    return bytes(pkt)


def GetChecksumStart(tc, obj, args=None):
    pkt = tc.packets.db['PKT1'].spktobj.spkt
    if pkt.haslayer(IP):
        return len(pkt) - len(pkt[IP].payload)
    elif pkt.haslayer(IPv6):
        return len(pkt) - len(pkt[IPv6].payload)
    else:
        raise NotImplementedError


def GetChecksumOffset(tc, obj, args=None):
    pkt = tc.packets.db['PKT1'].spktobj.spkt
    if pkt.haslayer(TCP):
        return 16
    elif pkt.haslayer(UDP):
        return 6
    elif pkt.haslayer(ICMP):
        return 2
    elif pkt.haslayer(IPv6) and pkt[IPv6].nh == 58: # ICMP
        # https://tools.ietf.org/html/rfc4443#section-2.3
        return 2
    else:
        raise NotImplementedError


TxOpcodeEnum = {
    'TXQ_DESC_OPCODE_CALC_NO_CSUM' : 0x0,
    'TXQ_DESC_OPCODE_CALC_CSUM' : 0x1,
    'TXQ_DESC_OPCODE_TSO': 0x2
}

def GetTxOpcodeCalcCsum(tc, obj, args=None):
    return TxOpcodeEnum['TXQ_DESC_OPCODE_CALC_CSUM']
