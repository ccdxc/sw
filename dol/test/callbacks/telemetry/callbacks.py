#! /usr/bin/python3
import pdb

class spanCbData:
    localExpPackets = {
        "SPAN_LOCAL_INGRESS_UPLINK": [
            ("SPAN_PKT1", "ingress", "session1")
            ],
        "SPAN_LOCAL_IGRESS_PC": [
            ("SPAN_PKT1", "ingress", "session3")
            ],
        "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
            ("SPAN_PKT1", "ingress", "session1"),
            ("SPAN_PKT1", "ingress", "session2")
            ],
        "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
            ("SPAN_PKT1", "ingress", "session1"),
            ("SPAN_PKT1", "ingress", "session3")
            ],
        "RSPAN_LOCAL_INGRESS_UPLINK": [
            ("RSPAN_PKT1", "ingress", "session1")
            ],
        "RSPAN_LOCAL_IGRESS_PC": [
            ("RSPAN_PKT1", "ingress", "session3")
            ],
        "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
            ("RSPAN_PKT1", "ingress", "session1"),
            ("RSPAN_PKT1", "ingress", "session2")
            ],
        "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
            ("RSPAN_PKT1", "ingress", "session1"),
            ("RSPAN_PKT1", "ingress", "session3")
            ],
        "SPAN_RSPAN_LOCAL_INGRESS_UPLINK": [
            ("SPAN_PKT1", "ingress", "session1"),
            ("RSPAN_PKT1", "ingress", "session2")
            ],
        "SPAN_RSPAN_LOCAL_IGRESS_PC": [
            ("SPAN_PKT1", "ingress", "session1"),
            ("RSPAN_PKT1", "ingress", "session3")
            ],
        "RSPAN_SPAN_LOCAL_IGRESS_PC": [
            ("RSPAN_PKT1", "ingress", "session1"),
            ("SPAN_PKT1", "ingress", "session3")
            ]
        }
    rspanExpPackets = {}
    erspanExpPackets = {}

    @staticmethod
    def getPacket(testcase, id):
        case = testcase.module.iterator.Get()
        if id > len(spanCbData.localExpPackets[case]):
            return None
        (pkt, dir, ssn) = spanCbData.localExpPackets[case][id - 1]
        return testcase.packets.Get(pkt)

    @staticmethod
    def getPort(testcase, id):
        case = testcase.module.iterator.Get()
        if id > len(spanCbData.localExpPackets[case]):
            return None
        (pkt, dir, ssn) = spanCbData.localExpPackets[case][id - 1]
        if dir == "ingress":
            return testcase.config.ingress_mirror.__dict__[ssn].intf.ports
        elif dir == "egress":
            return testcase.config.egress_mirror.__dict__[ssn].intf.ports
        return None

def GetExpectedPacket1(testcase):
    return spanCbData.getPacket(testcase, 1)

def GetExpectedPacket2(testcase):
    return spanCbData.getPacket(testcase, 2)

def GetExpectedPacket3(testcase):
    return spanCbData.getPacket(testcase, 3)

def GetExpectedPacket4(testcase):
    return spanCbData.getPacket(testcase, 4)

def GetExpectedPacket5(testcase):
    return spanCbData.getPacket(testcase, 5)

def GetExpectedPacket6(testcase):
    return spanCbData.getPacket(testcase, 6)

def GetExpectedPort1(testcase):
    return spanCbData.getPort(testcase, 1)

def GetExpectedPort2(testcase):
    return spanCbData.getPort(testcase, 2)

def GetExpectedPort3(testcase):
    return spanCbData.getPort(testcase, 3)

def GetExpectedPort4(testcase):
    return spanCbData.getPort(testcase, 4)

def GetExpectedPort5(testcase):
    return spanCbData.getPort(testcase, 5)

def GetExpectedPort6(testcase):
    return spanCbData.getPort(testcase, 6)

def GetRspanVlan(testcase, packet):
    return testcase.config.ingress_mirror.__dict__["session2"].segment.vlan_id
