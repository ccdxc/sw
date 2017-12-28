
from infra.common.logging import cfglogger
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
