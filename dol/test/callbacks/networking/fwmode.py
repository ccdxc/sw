#! /usr/bin/python3

def GetRxTriggerPorts(tc, args = None):
    assert(tc.config.src.endpoint.remote is True)
    if tc.config.src.tenant.IsHostPinned():
        return tc.config.dst.endpoint.pinintf.ports
    return tc.config.src.endpoint.intf.ports

def GetTxExpectedPorts(tc, args = None):
    assert(tc.config.dst.endpoint.remote is True)
    if tc.config.src.tenant.IsHostPinned():
        return tc.config.src.endpoint.pinintf.ports
    return tc.config.dst.endpoint.intf.ports

