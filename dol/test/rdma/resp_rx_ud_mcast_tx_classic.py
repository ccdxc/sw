#! /usr/bin/python3

from test.rdma.utils import *
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    tc.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data

    #Showing the UDQPs
    tc.info("Build Pruned UD QPs list for this Segement");
    tc.pvtdata.udqps_pruned_list = []
    segment = rs.session.responder.ep.segment
    # testcase/config/rdmasession/session/responder/ep/segment/floodlist/oifs/id=Enic7/lif
    #transmit_lif = rs.session.responder.ep.segment.floodlist.oifs.Get('Enic8').lif
    transmit_lif = rs.lqp.pd.ep.intf.lif
    tc.info("Transmit LIF: %s " % transmit_lif.GID());

    tc.info("    Number of EPs %d" % len(segment.GetEps()))
    for endpoint in segment.GetEps():
        # Uplinks to be ignored, as their validation verified by o/p pkt in the test
        # Uplinks will not have Lif, find and ignore them
        tc.info("    EP: %s LIF: %s UdQps: %d" % (endpoint.GID(),
                'None' if (not endpoint.intf.lif) else endpoint.intf.lif.GID(), len(endpoint.GetUdQps())))
        if (not endpoint.intf.lif):
            continue

        # Srcport pruning will not send mcast copies to lif where mcast pkt originated
        # so skip QPs with LIF same as sending LIF
        if endpoint.intf.lif == transmit_lif:
            continue
        # For now we need to post and validate for QPs with QPID less than 5, otherwise ignore
        for qp in endpoint.GetUdQps():
            tc.info("            EP: %s QP: %s " % (endpoint.GID(), qp.GID()))
            if qp.id > 5:
                continue
            #tc.pvtdata.udqps_pruned_list.append(endpoint.intf.lif, qp)
            pair = (endpoint.intf.lif, qp)
            tc.pvtdata.udqps_pruned_list.append(pair)
    tc.info("Pruned UD QPs list:")
    for lif, qp in tc.pvtdata.udqps_pruned_list:
       tc.info("   LIF: %s  QP: %s " % (lif.GID(), qp.GID()))

    return

def TestCaseTrigger(tc):
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):

    if (GlobalOptions.dryrun): return True

    tc.info("RDMA TestCaseVerify() Implementation.")

    return True  #TODO: For now return True for Validation


def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")
    return
