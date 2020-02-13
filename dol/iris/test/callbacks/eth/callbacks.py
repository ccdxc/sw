
import copy
import struct
import random
from itertools import islice
from socket import inet_aton
from scapy.utils import checksum
from scapy.layers.l2 import Ether, Dot1Q
from scapy.layers.inet import ICMP, TCP, UDP, IP
from scapy.layers.inet6 import in6_chksum, IPv6

import iris.config.hal.defs as haldefs
from factory.objects.eth.descriptor import *
from infra.common.logging import logger

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

def GetRxPktType(tc, obj, args=None):
    pkt = tc.packets.db['PKT2'].spktobj.spkt
    lif = tc.config.dst.endpoint.intf.lif
    flags = []
    if pkt.haslayer(IP):
        if pkt.haslayer(TCP):
            return LifRssType.Value("RSS_TYPE_IPV4") + LifRssType.Value("RSS_TYPE_IPV4_TCP")
        elif pkt.haslayer(UDP):
            return LifRssType.Value("RSS_TYPE_IPV4") + LifRssType.Value("RSS_TYPE_IPV4_UDP")
        else:
            return LifRssType.Value("RSS_TYPE_IPV4")
    elif pkt.haslayer(IPv6):
        if pkt.haslayer(TCP):
            return LifRssType.Value("RSS_TYPE_IPV6") + LifRssType.Value("RSS_TYPE_IPV6_TCP")
        elif pkt.haslayer(UDP):
            return LifRssType.Value("RSS_TYPE_IPV6") + LifRssType.Value("RSS_TYPE_IPV6_UDP")
        else:
            return LifRssType.Value("RSS_TYPE_IPV6")
    return LifRssType.Value("RSS_TYPE_NONE")


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
    elif pkt.haslayer(IPv6):
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
    'TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP' : 0x2,
    'TXQ_DESC_OPCODE_TSO': 0x3
}


def GetTxOpcodeCalcCsum(tc, obj, args=None):
    return TxOpcodeEnum['TXQ_DESC_OPCODE_CALC_CSUM']


def GetTxOpcodeCalcCsumTcpUdp(tc, obj, args=None):
    return TxOpcodeEnum['TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP']


def GetL2ChecksumCalc(tc, obj, args=None):
    spktobj = tc.packets.db['EXP_PKT'].spktobj
    pkt = spktobj.spkt
    return 1 if GetIsIP(tc, obj, args) else 0


def GetL2Checksum(tc, obj, args=None):
    spktobj = tc.packets.db['EXP_PKT'].spktobj
    pkt = spktobj.spkt
    if not GetIsIP(tc, obj, args):
        return 0
    chksum = (checksum(bytes(pkt[Ether].payload)) ^ -1) & 0xffff
    return struct.unpack('<H', struct.pack('>H', chksum))[0]


def GetIsIPV4(tc, obj, args=None):
    return tc.config.flow.IsIPV4()


def GetIsIPV6(tc, obj, args=None):
    return tc.config.flow.IsIPV6()


def GetIsTCP(tc, obj, args=None):
    return tc.config.flow.IsTCP()


def GetIsUDP(tc, obj, args=None):
    return tc.config.flow.IsUDP()


def GetIsICMP(tc, obj, args=None):
    return tc.config.flow.IsICMP() or tc.config.flow.IsICMPV6()


def GetIsTCPUDPICMP(tc, obj, args=None):
    return 1 if GetIsTCP(tc, obj, args) or GetIsUDP(tc, obj, args=None) or \
            GetIsICMP(tc, obj, args=None) else 0


def GetIsIP(tc, obj, args):
    return tc.config.flow.IsIP()


def InitEthRxSgDescriptor(tc, obj, args=None):
    desc = tc.descriptors.db[obj.desc]
    buf = desc._buf
    nfrags = int(obj.nfrags)

    if nfrags == 0:
        return

    addr = buf.addr
    bytes_rem = buf.size
    frag_sz = bytes_rem // (nfrags + 1)

    # Init Descriptor
    desc.fields = dict()
    desc.fields['addr'] = addr
    desc.fields['len'] = frag_sz
    addr += frag_sz
    bytes_rem -= frag_sz

    # Init SG elements
    desc._sgelems = [None] * nfrags

    for i in range(nfrags - 1):
        desc._sgelems[i] = EthRxSgElement(addr=addr, len=frag_sz)
        addr += frag_sz
        bytes_rem -= frag_sz

    # last frag will contain remainder bytes
    desc._sgelems[nfrags-1] = EthRxSgElement(addr=addr, len=bytes_rem)
    addr += frag_sz

    logger.info("Init %s" % desc)


def InitEthTxSgDescriptor(tc, obj, args=None):
    desc = tc.descriptors.db[obj.desc]
    buf = tc.buffers.db[obj.buf]
    pkt = tc.packets.db[obj.pkt].spktobj.spkt
    nfrags = int(obj.nfrags)

    if nfrags == 0:
        return

    if pkt.haslayer(UDP):
        hdr_len = len(pkt) - len(pkt[UDP].payload)
    elif pkt.haslayer(TCP):
        hdr_len = len(pkt) - len(pkt[TCP].payload)
    else:
        desc.addr = buf.addr
        desc.len = len(pkt)
        return

    # Init Descriptor
    desc.fields = dict()
    desc.fields['addr'] = buf.addr
    desc.fields['len'] = hdr_len
    desc.fields['num_sg_elems'] = nfrags
    desc._sgelems = [None] * nfrags

    # Init SG elements
    addr = buf.addr + hdr_len
    bytes_rem = len(pkt) - hdr_len
    frag_sz = bytes_rem // nfrags

    for i in range(nfrags - 1):
        desc._sgelems[i] = EthTxSgElement(addr=addr, len=frag_sz)
        addr += frag_sz
        bytes_rem -= frag_sz

    # last frag will contain remainder bytes
    desc._sgelems[nfrags-1] = EthTxSgElement(addr=addr, len=bytes_rem)
    addr += frag_sz

    logger.info("Init %s" % desc)


def InitEthTxTsoDescriptor(tc, obj, args=None):
    buf = tc.buffers.db[obj.buf]
    pkt = tc.packets.db[obj.pkt].spktobj.spkt
    name = obj.id
    mss = int(obj.mss)
    nfrags = int(obj.nfrags)

    assert buf.size >= len(pkt)
    assert pkt.haslayer(TCP)

    descriptors = list()

    hdr_len = len(pkt) - len(pkt[TCP].payload)
    bytes_rem = len(pkt)    # number of bytes remaining in the buffer
    addr = buf.addr     # current pointer into the buffer

    sot, eot = True, False
    i = 1
    while not eot:
        eot = bytes_rem <= mss

        # example tx descriptors from real usage
        # 00000330  34 e1 ce 9e fd 0f 00 00  42 00 00 00 42 00 a8 05
        #     (op=TSO, flags=SOT, nsge=1, addr=0xffd9ecee, len=0x42, hdrlen=0x42, mss=0x5a8)
        #     (+0x5a8 bytes in sge, total len=0x5ea)
        #
        # 00000340  30 80 01 9d fd 0f 00 00  a8 05 00 00 42 00 a8 05
        #     (op=TSO, flags=0, nsge=0, addr=0xffd9d018, len=0x5a8, hdrlen=0x42, mss=0x5a8)
        #
        # 00000350  30 00 5c 9c fd 0f 00 00  a8 05 00 00 42 00 a8 05
        #     (op=TSO, flags=0, nsge=0, addr=0xffd9c5c0, len=0x5a8, hdrlen=0x42, mss=0x5a8)
        #
        # 00000360  30 80 b6 d4 da 0f 00 00  a8 05 00 00 42 00 a8 05
        #     (op=TSO, flags=0, nsge=0, addr=0xfdad4b68, len=0x5a8, hdrlen=0x42, mss=0x5a8)
        #
        # 00000370  38 00 11 9b fd 0f 00 00  a8 05 00 00 42 00 a8 05
        #     (op=TSO, flags=EOT, nsge=0, addr=0xffd9b110, len=0x5a8, hdrlen=0x42, mss=0x5a8)

        desc = EthTxDescriptorObject()
        desc.GID('%s_%d' % (name, i))
        desc.fields = dict()
        desc.fields['opcode'] = TxOpcodeEnum['TXQ_DESC_OPCODE_TSO']
        desc.fields['csum_l3_or_sot'] = sot
        desc.fields['csum_l4_or_eot'] = eot
        desc.fields['csum_start_or_hdr_len'] = hdr_len
        desc.fields['csum_offset_or_mss'] = mss
        desc.fields['addr'] = addr

        if sot:
            # At start of TSO we should have enough bytes in the buffer to form
            # atleast the header.
            assert bytes_rem >= hdr_len
            # The start of TSO descriptor points to header + one segment of data
            seg_len = min(hdr_len + mss, bytes_rem)
        else:
            # All tso descriptors must point to atleast one byte of data
            assert bytes_rem > 0
            # All tso descriptors except the start of tso descriptor points to
            # one segment of data
            seg_len = min(mss, bytes_rem)

        seg_bytes_rem = seg_len

        if nfrags == 0:
            desc.fields['num_sg_elems'] = 0
            desc.fields['len'] = seg_bytes_rem
            addr += seg_bytes_rem
            seg_bytes_rem -= seg_bytes_rem
        else:
            # we should be able to get atleast one byte per fragment
            assert seg_bytes_rem >= (nfrags + 1)
            desc.fields['num_sg_elems'] = nfrags
            desc._sgelems = [None] * nfrags
            # print('DESC%d' % i, nfrags)
            if sot:
                frag_sz = (seg_bytes_rem - hdr_len) // nfrags
                desc.fields['len'] = hdr_len    # header is indivisible
                seg_bytes_rem -= hdr_len
                addr += hdr_len
            else:
                frag_sz = seg_bytes_rem // (nfrags + 1)
                desc.fields['len'] = frag_sz
                seg_bytes_rem -= frag_sz
                addr += frag_sz

            frag_idx = 0
            while seg_bytes_rem > 0:
                frag_sz = seg_bytes_rem if frag_idx == nfrags - 1 else frag_sz
                # print('DESC%d/FRAG%d' % (i, frag_idx))
                desc._sgelems[frag_idx] = EthTxSgElement(addr=addr, len=frag_sz)
                frag_idx += 1
                seg_bytes_rem -= frag_sz
                addr += frag_sz

        # we should have used up all the bytes from the segment after we
        # are done encoding this descriptor
        assert seg_bytes_rem == 0
        bytes_rem -= seg_len

        logger.info("Init %s" % desc)

        descriptors.append(desc)
        i += 1
        sot = 0

    return descriptors


def InitEthTxTsoPackets(tc, obj, args=None):
    pktobj = tc.packets.db[obj.pkt]
    pkt = pktobj.spktobj.spkt
    mss = int(obj.mss)

    assert pkt.haslayer(TCP)
    hdr_len = len(pkt) - len(pkt[TCP].payload)

    packets = list()

    bytes_off = 0
    bytes_rem = len(pkt[TCP].payload)

    logger.info("Segmenting %s len %d" % (pktobj.GID(), len(pktobj)))

    i = 1
    while bytes_rem:
        if bytes_rem <= mss:
            seg_len = bytes_rem
        else:
            seg_len = mss

        segobj = copy.deepcopy(pktobj)
        segobj.GID('EXP_PKT%d' % i)

        segpkt = segobj.spktobj.spkt
        segpkt[IP].id += i - 1
        segpkt[TCP].payload.data = pkt[TCP].payload.data[bytes_off:bytes_off+seg_len]
        segpkt[TCP].seq += bytes_off

        segobj.headers.payload.meta.size = seg_len
        segobj.payloadsize = seg_len
        segobj.spktobj.rawbytes = bytes(segpkt)
        segobj.rawbytes = segobj.spktobj.rawbytes

        logger.info("Init %s len %d" % (segobj.GID(), len(segobj)))

        packets.append(segobj)

        # next pkt
        bytes_off += seg_len
        bytes_rem -= seg_len
        i += 1

    return packets


def GenPayload():
    '''Generate the bytes 0..255 followed by prng bytes seeded with zero'''
    # start with 0..255
    for x in range(256):
        yield x
    # then prng bytes seeded with zero
    r = random.Random(0)
    while True:
        yield r.randrange(0, 256)

def GetPayload(tc, obj, args=None):
    return list(islice(GenPayload(), args.size))

def Debug(tc, obj=None, args=None):
    pass
