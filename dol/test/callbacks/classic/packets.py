#! /usr/bin/python3
import pdb

from infra.common.glopts import GlobalOptions as GlobalOptions

def IsPriorityTagged(pvtdata):
    if 'priotag' in pvtdata.__dict__:
        return pvtdata.priotag
    return False

def __get_packet_encap_vlan(testcase, cfg):
    assert(GlobalOptions.classic)

    if cfg.endpoint == None:
        return None

    if cfg.endpoint.IsNative():
        return None

    return __get_template('ENCAP_QTAG')

def __get_packet_encap_vxlan(testcase, cfg):
    if cfg.endpoint.remote == False:
        return None
    if cfg.segment.IsFabEncapVxlan():
        return __get_template('ENCAP_VXLAN')
    return None

def __get_packet_encaps(testcase, cfg):
    encaps = []

    # Check for VLAN encap
    encap = __get_packet_encap_vlan(testcase, cfg)
    if encap:
        encaps.append(encap)

    # Check for VXLAN encap
    encap = __get_packet_encap_vxlan(testcase, cfg)
    if encap:
        encaps.append(encap)

    if len(encaps):
        return encaps
    return None

def GetExpectedPacketEncaps(testcase, packet):
    ptag = IsPriorityTagged(testcase.pvtdata)
    testcase.pvtdata.priotag = False
    encaps = __get_packet_encaps(testcase, testcase.config.dst)
    testcase.pvtdata.priotag = ptag
    return encaps

