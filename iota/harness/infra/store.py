#! /usr/bin/python3
import pdb
import copy

import iota.harness.infra.types as types

__gl_testbed = None
__gl_topology = None
__gl_workloads = {}

def SetTestbed(tb):
    global __gl_testbed
    assert(tb)
    __gl_testbed = tb
    return

def GetTestbed():
    global __gl_testbed
    assert(__gl_testbed)
    return __gl_testbed

def SetTopology(topo):
    global __gl_topology
    assert(topo)
    __gl_topology = topo
    return

def GetTopology():
    global __gl_topology
    assert(__gl_topology)
    return __gl_topology

class Workload:
    def __init__(self, msg):
        self.workload_name = msg.workload_name
        self.node_name = msg.node_name
        self.encap_vlan = msg.encap_vlan
        self.ip_prefix = msg.ip_prefix
        self.ip_address = msg.ip_prefix.split('/')[0]
        self.mac_address = msg.mac_address
        self.interface = msg.interface
        self.interface_type = msg.interface_type
        self.pinned_port = msg.pinned_port
        self.uplink_vlan = msg.uplink_vlan
        return

    def IsNaples(self):
        return GetTestbed().GetCurrentTestsuite().GetTopology().GetNicType(self.node_name) == 'pensando'

def AddWorkloads(req):
    global __gl_workloads
    for wlmsg in req.workloads:
        wl = Workload(wlmsg)
        __gl_workloads[wl.workload_name] = wl
    return

def GetWorkloads():
    global __gl_workloads
    return __gl_workloads.values()
    
def DeleteWorkloads(req):
    global __gl_workloads
    for wl in req.workloads:
        del __gl_workloads[wl.workload_name]
    return

def GetLocalWorkloadPairs():
    pairs = []
    for w1 in GetWorkloads():
        for w2 in GetWorkloads():
            if id(w1) == id(w2): continue
            if w1.uplink_vlan != w2.uplink_vlan: continue
            if w1.node_name == w2.node_name:
                pairs.append((w1, w2))
    return pairs

def GetRemoteWorkloadPairs():
    pairs = []
    for w1 in GetWorkloads():
        for w2 in GetWorkloads():
            if id(w1) == id(w2): continue
            if w1.uplink_vlan != w2.uplink_vlan: continue
            if w1.node_name != w2.node_name:
                pairs.append((w1, w2))
    return pairs

def SetVeniceConfigs(json_objs):
    global __gl_venice_configs
    __gl_venice_configs = json_objs
    return types.status.SUCCESS

def GetVeniceConfigs():
    global __gl_venice_configs
    return __gl_venice_configs

def SetVeniceAuthToken(auth_token):
    global __gl_venice_auth_token
    __gl_venice_auth_token = auth_token
    return types.status.SUCCESS
    
def GetVeniceAuthToken():
    return __gl_venice_auth_token

def Cleanup():
    global __gl_workloads
    global __gl_venice_configs
    global __gl_venice_auth_token
    __gl_workloads = {}
    __gl_venice_configs = None
    __gl_venice_auth_token = None
