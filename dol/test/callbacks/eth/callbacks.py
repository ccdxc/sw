
from infra.common.logging import cfglogger
import config.hal.defs as haldefs
from bitstring import BitArray

from .toeplitz import *


LifRssType = haldefs.interface.LifRssType


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
