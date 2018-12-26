#! /usr/bin/python3
import iota.harness.api as api

def GetThreeWorkloads():
    triplet = []
    for w1 in api.GetWorkloads():
        for w2 in api.GetWorkloads():
            for w3 in api.GetWorkloads():
                if id(w1) == id(w2) or \
                   id(w2) == id(w3) or id(w1) == id(w3): continue
                if w1.uplink_vlan != w2.uplink_vlan or \
                   w2.uplink_vlan != w3.uplink_vlan or \
                   w1.uplink_vlan != w3.uplink_vlan: continue
                triplet.append((w1, w2, w3))
    return triplet
