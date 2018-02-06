#!/usr/bin/python3

import zmq
import os
import sys
import pdb

sys.path.append('model_sim/src/')

import model_wrap

from scapy.all import *


def packet_to_bytes(pkt):
    ba = [hex(x) for x in bytes(pkt)]
    s = str(ba)
    s = s.replace('[', '{')
    s = s.replace(']', '}')
    s = s.replace("'", "")
    return s


uplink = {
    1: {
        "mac": "00:ee:ff:00:00:02",
        "ip": "64.0.0.1",
    },
    2: {
        "mac": "00:ee:ff:00:00:03",
        "ip": "64.0.0.2",
    },
    3: {
        "mac": "00:ee:ff:00:00:04",
        "ip": "64.0.0.3",
    },
    4: {
        "mac": "00:ee:ff:00:00:05",
        "ip": "64.0.0.4",
    },
}

enic = {
    7: {
        "mac": "00:ee:00:00:00:02",
        "ip": "64.0.0.5",
        "vlan": 3001,
    },
    8: {
        "mac": "00:ee:00:00:00:03",
        "ip": "64.1.0.5",
        "vlan": 3002,
    }
}

print("""
#ifndef __FLOWS_HPP__
#define __FLOWS_HPP__
"""
)

# Enic7->Uplink1
pkt = Ether(src=enic[7]["mac"], dst=uplink[1]["mac"]) / \
      Dot1Q(vlan=enic[7]["vlan"]) / \
      IP(src=enic[7]["ip"], dst="64.1.0.1") / \
      TCP(sport=42002, dport=42001) / \
      Raw(load="Pensando Systems")
print("uint8_t enic_to_uplink[] = " + packet_to_bytes(pkt) + ";")

# Uplink1->Enic7
pkt = Ether(src=uplink[1]["mac"], dst=enic[7]["mac"]) / \
      Dot1Q(vlan=enic[7]["vlan"]) / \
      IP(src=uplink[1]["ip"], dst=enic[7]["ip"]) / \
      TCP(sport=42001, dport=42002) / \
      Raw(load="Pensando Systems")
print("uint8_t uplink_to_enic[] = " + packet_to_bytes(pkt) + ";")

# Enic7->Enic8
pkt = Ether(src=enic[7]["mac"], dst=enic[8]["mac"]) / \
      Dot1Q(vlan=enic[7]["vlan"]) / \
      IP(src=enic[7]["ip"], dst=enic[8]["ip"]) / \
      TCP(sport=42001, dport=42002) / \
      Raw(load="Pensando Systems")
print("uint8_t enic_to_enic[] = " + packet_to_bytes(pkt) + ";")

print("""
#endif /// __FLOWS_HPP__
"""
)

