#!/usr/bin/python3

from enum import IntEnum
from scapy.layers.inet import IP, TCP, UDP
from scapy.layers.inet6 import IPv6
from bitstring import BitArray
from socket import inet_pton, AF_INET, AF_INET6
from copy import deepcopy
from binascii import hexlify
import socket


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


def toeplitz_input(dip, sip, dport, sport, rss_type_num):
    inp = BitArray()

    if rss_type_num in [RSS.IPV4, RSS.IPV4_TCP, RSS.IPV4_UDP]:
        inp += BitArray(uint=ipv4_to_int(sip), length=32)
        inp += BitArray(uint=ipv4_to_int(dip), length=32)
    elif rss_type_num in [RSS.IPV6, RSS.IPV6_TCP, RSS.IPV6_UDP]:
        inp += BitArray(uint=ipv6_to_int(sip), length=128)
        inp += BitArray(uint=ipv6_to_int(dip), length=128)

    if rss_type_num in [RSS.IPV4_TCP, RSS.IPV6_TCP]:
        inp += BitArray(uint=sport, length=16)
        inp += BitArray(uint=dport, length=16)
    elif rss_type_num in [RSS.IPV4_UDP, RSS.IPV6_UDP]:
        inp += BitArray(uint=sport, length=16)
        inp += BitArray(uint=dport, length=16)

    return inp
