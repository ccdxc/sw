#! /usr/bin/python3

def GetTxExpectedPacket(tc, args = None):
    return tc.packets.Get(args.expktid)

def GetRxExpectedPacket(tc, args = None):
    return tc.packets.Get(args.expktid)

def GetTxExpectedPorts(tc, args = None):
    assert(tc.config.dst.endpoint.remote is True)
    return tc.config.src.intf.pinnedif.ports

def GetRxTriggerPorts(tc, args = None):
    assert(tc.config.src.endpoint.remote is True)
    return tc.config.dst.endpoint.pinintf.ports

def GetCpuPacket(tc, args = None):
    return tc.packets.Get(args.expktid)
