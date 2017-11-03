#! /usr/bin/python3

def GetRxTriggerPorts(tc, args = None):
    assert(tc.config.src.endpoint.remote is True)
    if tc.pvtdata.scenario != 'RPF_FAILURE':
        if tc.config.src.tenant.IsHostPinned():
            return tc.config.dst.endpoint.pinintf.ports
        return tc.config.src.endpoint.intf.ports

    # RPF Failure testcase.
    pin_ports = tc.config.dst.endpoint.pinintf.ports
    rpf_fail_ports = set([1, 2, 3, 4]) - set(pin_ports)
    return list(rpf_fail_ports)[:1]

def GetTxExpectedPorts(tc, args = None):
    assert(tc.config.dst.endpoint.remote is True)
    if tc.config.src.tenant.IsHostPinned():
        return tc.config.src.endpoint.pinintf.ports
    return tc.config.dst.endpoint.intf.ports

def GetRxDstIpAddress(testcase, packet):
    if testcase.pvtdata.scenario != 'UNKNOWN_DEST_EP':
        return testcase.config.flow.dip

    if testcase.config.flow.IsIPV4():
        return '4.3.2.1'
    elif testcase.config.flow.IsIPV6():
        return '4000::1'
    
    assert(0)
