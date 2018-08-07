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

def __get_qtag(pri = 0, vlan = 0):
    qtag = (pri << 13) + vlan
    assert(qtag <= 0xffff)
    return qtag

def GetExpectedPacketQtag(testcase, args=None):
    assert(testcase.config.dst.endpoint.remote == False)    # Always Host RX
    if testcase.config.src.segment.native is False or IsPriorityTagged(testcase.pvtdata):
        pri = testcase.config.flow.txqos.cos
        vlan_id = testcase.config.dst.endpoint.intf.encap_vlan_id
        return __get_qtag(pri, vlan_id)
    return __get_qtag()

def GetExpectedMcastPacketQtag(testcase, args = None):
    pri = testcase.config.flow.txqos.cos
    vlan_id = testcase.config.src.segment.vlan_id
    return __get_qtag(pri, vlan_id)
