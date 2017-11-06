#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
from config.store import Store
import config.objects.span as span
import config.objects.tunnel as tunnel

erspan_sessions = []

class spanSessionData:
    localSpanSsns = {
            "SPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0)
                ],
            "SPAN_LOCAL_INGRESS_PC": [
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0)
                ],
            "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0)
                ],
            "RSPAN_LOCAL_INGRESS_PC": [
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0)
                ],
            "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT2", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_SPAN_ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0)
                ],
            "SPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "LOCAL", "SPAN_PKT1", "Uplink2", 0)
                ],
            "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0)
                ],
            "RSPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0)
                ],
            "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "ERSPAN", "ERSPAN_PKT2", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "egress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_EGRESS_PC": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "egress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_SPAN_LOCAL_EGRESS_PC": [
                (1, "egress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_SPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "egress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_INGRESS_LOCAL_EP": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 0),
            ],
            "SPAN_LOCAL_INGRESS_UPLINK_SNAP100": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 100),
                ],
            "L3_SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_PC": [
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0)
                ],
            "L3_ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT2", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_SPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "LOCAL", "EG_SPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink2", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0)
                ]
    }

    def __init__(self):
        self._cur_erspan_sessions = []
        self.erspan_count = 0

    @staticmethod
    def getSessions(case):
        return spanSessionData.localSpanSsns[case]

    def getErspanSession(self, id):
        if id <= len(self._cur_erspan_sessions):
            return self._cur_erspan_sessions[id - 1]
        return None

    def addErspanSession(self, tnl):
        self._cur_erspan_sessions.append(tnl)

    def clearErspanSession(self):
       self._cur_erspan_sessions = []

def clearAllSessions():
    spans = Store.objects.GetAllByClass(span.SpanSessionObject)
    for ssn in Store.objects.GetAllByClass(span.SpanSessionObject):
        ssn.Clear()
    erspan_sessions = []
    data.erspan_count = 0
    return

def setup_span(infra, module, case):
    clearAllSessions()
    ssns = spanSessionData.getSessions(case)
    # setup default ERSPAN tunnels
    count = 0
    for tnl in Store.objects.GetAllByClass(tunnel.TunnelObject):
        if tnl.IsGRE():
            data.addErspanSession(tnl)
            count = count + 1

    count = 0
    for (id, direc, spantype, pkt, intf, pktlen) in ssns:
        ofset = 0
        #if direc == "egress":
        #    ofset = 3
        id = id + ofset
        sessname = "SpanSession000" + str(id)
        sess = infra.ConfigStore.objects.Get(sessname)
        if spantype == "ERSPAN":
            count = 0
            for tnl in Store.objects.GetAllByClass(tunnel.TunnelObject):
                if tnl.IsGRE():
                    if count == data.erspan_count:
                        sess.erspan_dest = tnl.GetDestIp()
                        sess.erspan_src = tnl.GetSrcIp()
                        sess.span_id = 0
                        data.addErspanSession(tnl)
                        data.erspan_count = data.erspan_count + 1
                        break
                    count = count + 1

        if intf == "AnyLocalEP":
            ten = infra.ConfigStore.objects.Get("Ten0001")
            eps = ten.GetLocalEps()
            dintf = eps[0].intf 
            #pdb.set_trace()
        else:
            dintf = infra.ConfigStore.objects.Get(intf)
        module.logger.info("Updating Span Session %s on interface %s with type %s" % (sessname, intf, spantype))
        snaplen = 0
        if pktlen != 0:
            snaplen = pktlen - 14
            if pktlen < 14:
                snaplen = 1
        sess.Update(snaplen, spantype, dintf)
    return

def Setup(infra, module):
    #pdb.set_trace()
    data.clearErspanSession()
    setup_span(infra, module, module.iterator.Get())
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    span_case = tc.module.iterator.Get()
    tc.pvtdata.span_case = span_case
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return

data = spanSessionData()
