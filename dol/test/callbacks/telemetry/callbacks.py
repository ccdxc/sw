#! /usr/bin/python3
import pdb

class spanCbData:
    localExpPackets = {
        "SPAN_LOCAL_INGRESS_UPLINK": [
            ("SPAN_PKT1", "ingress", "session1")
            ],
        "SPAN_LOCAL_INGRESS_PC": [
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
        "RSPAN_LOCAL_INGRESS_PC": [
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
        "SPAN_RSPAN_LOCAL_INGRESS_PC": [
            ("SPAN_PKT1", "ingress", "session1"),
            ("RSPAN_PKT1", "ingress", "session3")
            ],
        "RSPAN_SPAN_LOCAL_INGRESS_PC": [
            ("RSPAN_PKT1", "ingress", "session1"),
            ("SPAN_PKT1", "ingress", "session3")
            ],
        "SPAN_LOCAL_EGRESS_UPLINK": [
            ("SPAN_PKT1", "egress", "session1"),
            ],
        "SPAN_LOCAL_EGRESS_PC": [
            ("SPAN_PKT1", "egress", "session3"),
            ],
        "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
            ("SPAN_PKT1", "egress", "session1"),
            ("SPAN_PKT1", "egress", "session2")
            ],
        "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
            ("SPAN_PKT1", "egress", "session1"),
            ("SPAN_PKT1", "egress", "session3")
            ],
        "RSPAN_LOCAL_EGRESS_UPLINK": [
            ("RSPAN_PKT1", "egress", "session1")
            ],
        "RSPAN_LOCAL_EGRESS_PC": [
            ("RSPAN_PKT1", "egress", "session3")
            ],
        "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
            ("RSPAN_PKT1", "egress", "session1"),
            ("RSPAN_PKT1", "egress", "session2")
            ],
        "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
            ("RSPAN_PKT1", "egress", "session1"),
            ("RSPAN_PKT1", "egress", "session3")
            ],
        "SPAN_RSPAN_LOCAL_EGRESS_UPLINK": [
            ("SPAN_PKT1", "egress", "session1"),
            ("RSPAN_PKT1", "egress", "session2")
            ],
        "SPAN_RSPAN_LOCAL_EGRESS_PC": [
            ("SPAN_PKT1", "egress", "session1"),
            ("RSPAN_PKT1", "egress", "session3")
            ],
        "RSPAN_SPAN_LOCAL_EGRESS_PC": [
            ("RSPAN_PKT1", "egress", "session1"),
            ("SPAN_PKT1", "egress", "session3")
            ],
        "RSPAN_SPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
            ("RSPAN_PKT1", "ingress", "session1"),
            ("SPAN_PKT1", "egress", "session3")
            ],
        "SPAN_RSPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
            ("SPAN_PKT1", "ingress", "session1"),
            ("RSPAN_PKT1", "egress", "session3")
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

def GetExpectedPacket(testcase, args):
    return spanCbData.getPacket(testcase, args.sessionid)

def GetExpectedPort(testcase, args):
    return spanCbData.getPort(testcase, args.sessionid)

def GetRspanVlan(testcase, packet):
    return testcase.config.ingress_mirror.__dict__["session2"].segment.vlan_id

