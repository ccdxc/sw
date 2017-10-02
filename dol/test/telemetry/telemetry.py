#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
from config.store import Store
import config.objects.span as span

class spanSessionData:
    localSpanSsns = {
            "SPAN_LOCAL_INGRESS_UPLINK": [
                (1, "LOCAL", "Uplink1")
                ],
            "SPAN_LOCAL_INGRESS_PC": [
                (3, "LOCAL", "UplinkPc1")
                ],
            "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "LOCAL", "Uplink1"),
                (2, "LOCAL", "Uplink2")
                ],
            "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "LOCAL", "Uplink1"),
                (3, "LOCAL", "UplinkPc1")
                ],
            "RSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "RSPAN", "Uplink1")
                ],
            "RSPAN_LOCAL_INGRESS_PC": [
                (3, "RSPAN", "UplinkPc1")
                ],
            "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "RSPAN", "Uplink1"),
                (2, "RSPAN", "Uplink2")
                ],
            "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "RSPAN", "Uplink1"),
                (3, "RSPAN", "UplinkPc1")
                ],
            "SPAN_RSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "LOCAL", "Uplink1"),
                (2, "RSPAN", "Uplink2")
                ],
            "SPAN_RSPAN_LOCAL_INGRESS_PC": [
                (1, "LOCAL", "Uplink1"),
                (3, "RSPAN", "UplinkPc1")
                ],
            "RSPAN_SPAN_LOCAL_INGRESS_PC": [
                (1, "RSPAN", "Uplink1"),
                (3, "LOCAL", "UplinkPc1")
                ],
            "SPAN_LOCAL_EGRESS_UPLINK": [
                (1, "LOCAL", "Uplink1")
                ],
            "SPAN_LOCAL_EGRESS_PC": [
                (3, "LOCAL", "UplinkPc1")
                ],
            "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "LOCAL", "Uplink1"),
                (2, "LOCAL", "Uplink2")
                ],
            "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "LOCAL", "Uplink1"),
                (3, "LOCAL", "UplinkPc1")
                ],
            "RSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "RSPAN", "Uplink1")
                ],
            "RSPAN_LOCAL_EGRESS_PC": [
                (3, "RSPAN", "UplinkPc1")
                ],
            "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "RSPAN", "Uplink1"),
                (2, "RSPAN", "Uplink2")
                ],
            "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "RSPAN", "Uplink1"),
                (3, "RSPAN", "UplinkPc1")
                ],
            "SPAN_RSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "LOCAL", "Uplink1"),
                (2, "RSPAN", "Uplink2")
                ],
            "SPAN_RSPAN_LOCAL_EGRESS_PC": [
                (1, "LOCAL", "Uplink1"),
                (3, "RSPAN", "UplinkPc1")
                ],
            "RSPAN_SPAN_LOCAL_EGRESS_PC": [
                (1, "RSPAN", "Uplink1"),
                (3, "LOCAL", "UplinkPc1")
                ],
            "RSPAN_SPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
                (1, "RSPAN", "Uplink1"),
                (3, "LOCAL", "UplinkPc1")
                ],
            "SPAN_RSPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
                (1, "LOCAL", "Uplink1"),
                (3, "RSPAN", "UplinkPc1")
                ]

    }

    @staticmethod
    def getSessions(case):
        return spanSessionData.localSpanSsns[case]

def clearAllSessions():
    spans = Store.objects.GetAllByClass(span.SpanSessionObject)
    for ssn in Store.objects.GetAllByClass(span.SpanSessionObject):
        ssn.Clear()
    return

def setup_span(infra, module, case):
    clearAllSessions()
    ssns = spanSessionData.getSessions(case)
    for (id, spantype, intf) in ssns:
        sessname = "SpanSession000" + str(id)
        sess = infra.ConfigStore.objects.Get(sessname)
        dintf = infra.ConfigStore.objects.Get(intf)
        module.logger.info("Updating Span Session %s on interface %s with type %s", sessname, intf, spantype)
        sess.Update(0, spantype, dintf)
    return

def Setup(infra, module):
    #pdb.set_trace()
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
