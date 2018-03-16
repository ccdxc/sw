#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy
from config.objects.proxycb_service import ProxyCbServiceHelper

def get_tcbs(tc):
    id1, id2 = ProxyCbServiceHelper.GetSessionQids(tc.config.flow._FlowObject__session)
    if tc.config.flow.IsIflow():
        id = id1
        other_fid = id2
    else:
        id = id2
        other_fid = id1

    tcbid1 = "TcpCb%04d" % id
    tcbid2 = "TcpCb%04d" % other_fid
    print("%s is flow 1" % tcbid1)
    print("%s is flow 2" % tcbid2)

    tcb1 = tc.infra_data.ConfigStore.objects.db[tcbid1]
    tcb2 = tc.infra_data.ConfigStore.objects.db[tcbid2]

    tc.pvtdata.tcb1 = tcb1
    tc.pvtdata.tcb2 = tcb2

    return tcb1, tcb2

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)

    tcp_proxy.SetupProxyArgs(tc)

    tcb1, tcb2 = get_tcbs(tc)

    tcp_proxy.init_flow_pvtdata(tc, tcb1, tcb2)

    return

def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)
    return

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def TestCaseStepVerify(tc, step):
    return modcbs.TestCaseStepVerify(tc, step)

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
