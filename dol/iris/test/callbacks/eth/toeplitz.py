
import sys
from enum import IntEnum
from scapy.layers.inet import IP, TCP, UDP
from scapy.layers.inet6 import IPv6
from bitstring import BitArray
from socket import inet_pton, AF_INET, AF_INET6
from copy import deepcopy
from binascii import hexlify


def ipv4_to_int(addr):
    return int(hexlify(inet_pton(AF_INET, addr)), 16)


def ipv6_to_int(addr):
    return int(hexlify(inet_pton(AF_INET6, addr)), 16)


class RSS(IntEnum):
    NONE = 0
    IPV4 = 1
    IPV4_TCP = 2
    IPV4_UDP = 3
    IPV6 = 4
    IPV6_TCP = 5
    IPV6_UDP = 6
    INVALID = 7


toeplitz_msft_key = [
    0x6d, 0x5a, 0x56, 0xda, 0x25, 0x5b, 0x0e, 0xc2,
    0x41, 0x67, 0x25, 0x3d, 0x43, 0xa3, 0x8f, 0xb0,
    0xd0, 0xca, 0x2b, 0xcb, 0xae, 0x7b, 0x30, 0xb4,
    0x77, 0xcb, 0x2d, 0xa3, 0x80, 0x30, 0xf2, 0x0c,
    0x6a, 0x42, 0xb7, 0x3b, 0xbe, 0xac, 0x01, 0xfa,
]

toeplitz_symmetric_key = [
    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
]


toeplitz_default_key = [x & 0xff for x in range(40)]


def toeplitz_hash(inp, key):
    assert isinstance(inp, BitArray) and isinstance(key, BitArray)
    assert len(inp) < len(key)

    inp = deepcopy(inp)
    key = deepcopy(key)

    result = 0
    for b in inp.bin:
        if b == '1':
            result ^= key[:32].uint
        key.rol(1)

    return result


def toeplitz_input(rss_type_num, pkt):
    inp = BitArray()

    if rss_type_num in [RSS.IPV4, RSS.IPV4_TCP, RSS.IPV4_UDP]:
        inp += BitArray(uint=ipv4_to_int(pkt[IP].src), length=32)
        inp += BitArray(uint=ipv4_to_int(pkt[IP].dst), length=32)
    elif rss_type_num in [RSS.IPV6, RSS.IPV6_TCP, RSS.IPV6_UDP]:
        inp += BitArray(uint=ipv6_to_int(pkt[IPv6].src), length=128)
        inp += BitArray(uint=ipv6_to_int(pkt[IPv6].dst), length=128)

    if rss_type_num in [RSS.IPV4_TCP, RSS.IPV6_TCP]:
        inp += BitArray(uint=pkt[TCP].sport, length=16)
        inp += BitArray(uint=pkt[TCP].dport, length=16)
    elif rss_type_num in [RSS.IPV4_UDP, RSS.IPV6_UDP]:
        inp += BitArray(uint=pkt[UDP].sport, length=16)
        inp += BitArray(uint=pkt[UDP].dport, length=16)

    return inp


if __name__ == "__main__":

    #
    # Toeplitz Hash verification tests
    # Source - https://docs.microsoft.com/en-us/windows-hardware/drivers/network/verifying-the-rss-hash-calculation
    # Note: Unit Tests will run when this file is executed
    #

    __toeplitz_testflow = [
        IP(dst="161.142.100.80", src="66.9.149.187") / TCP(dport=1766, sport=2794),
        IP(dst="65.69.140.83", src="199.92.111.2") / TCP(dport=4739, sport=14230),
        IP(dst="12.22.207.184", src="24.19.198.95") / TCP(dport=38024, sport=12898),
        IP(dst="209.142.163.6", src="38.27.205.30") / TCP(dport=2217, sport=48228),
        IP(dst="202.188.127.2", src="153.39.163.191") / TCP(dport=1303, sport=44251),
        IPv6(dst="3ffe:2501:200:3::1", src="3ffe:2501:200:1fff::7") / TCP(dport=1766, sport=2794),
        IPv6(dst="ff02::1", src="3ffe:501:8::260:97ff:fe40:efab") / TCP(dport=4739, sport=14230),
        IPv6(dst="fe80::200:f8ff:fe21:67cf", src="3ffe:1900:4545:3:200:f8ff:fe21:67cf") / TCP(dport=38024, sport=44251)
    ]

    __toeplitz_testdata = [
        (__toeplitz_testflow[0], RSS.IPV4, 0x323e8fc2),
        (__toeplitz_testflow[0], RSS.IPV4_TCP, 0x51ccc178),
        (__toeplitz_testflow[1], RSS.IPV4, 0xd718262a),
        (__toeplitz_testflow[1], RSS.IPV4_TCP, 0xc626b0ea),
        (__toeplitz_testflow[2], RSS.IPV4, 0xd2d0a5de),
        (__toeplitz_testflow[2], RSS.IPV4_TCP, 0x5c2b394a),
        (__toeplitz_testflow[3], RSS.IPV4, 0x82989176),
        (__toeplitz_testflow[3], RSS.IPV4_TCP, 0xafc7327f),
        (__toeplitz_testflow[4], RSS.IPV4, 0x5d1809c5),
        (__toeplitz_testflow[4], RSS.IPV4_TCP, 0x10e828a2),

        (__toeplitz_testflow[5], RSS.IPV6, 0x2cc18cd5),
        (__toeplitz_testflow[5], RSS.IPV6_TCP, 0x40207d3d),
        (__toeplitz_testflow[6], RSS.IPV6, 0x0f0c461c),
        (__toeplitz_testflow[6], RSS.IPV6_TCP, 0xdde51bbf),
        (__toeplitz_testflow[7], RSS.IPV6, 0x4b61e985),
        (__toeplitz_testflow[7], RSS.IPV6_TCP, 0x02d1feef),
    ]

    for pkt, rss_type_num, exp_hash in __toeplitz_testdata:
        calc_hash = toeplitz_hash(toeplitz_input(rss_type_num, pkt), BitArray(bytes=toeplitz_msft_key))
        if calc_hash != exp_hash:
            pkt.show()
            print(rss_type_num)
            raise AssertionError("Hash Mismatch: Expected = 0x%x Calculated = 0x%x" % (exp_hash, calc_hash))
