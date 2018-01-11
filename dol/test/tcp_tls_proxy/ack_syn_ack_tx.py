# Testcase definition file.

import pdb
import copy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
from infra.common.objects import ObjectDatabase as ObjectDatabase
import test.callbacks.networking.modcbs as modcbs
from infra.common.logging import logger
from config.objects.cpucb        import CpuCbHelper
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy
import test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    tc.pvtdata = ObjectDatabase(logger)
    tcp_proxy.SetupProxyArgs(tc)

    id1, id2 = ProxyCbServiceHelper.GetSessionQids(tc.config.flow._FlowObject__session)
    if tc.config.flow.IsIflow():
        id = id1
        other_fid = id2
    else:
        id = id2
        other_fid = id1

    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcp_proxy.init_tcb_inorder(tc, tcb)
    tcb.debug_dol = 0
    # set tcb state to SYN_SENT(2)
    tcb.state = tcp_proxy.tcp_state_SYN_SENT
    tcb.SetObjValPd()

    TcpCbHelper.main(other_fid)
    tcbid2 = "TcpCb%04d" % (other_fid)
    tc.module.logger.info("Configuring %s" % tcbid2)
    tcb2 = tc.infra_data.ConfigStore.objects.db[tcbid2]
    tcp_proxy.init_tcb_inorder2(tc, tcb2)
    tcb2.SetObjValPd()



    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.GetMeta()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.GetMeta()
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmdr.GetMeta()
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])
    tnmpr.GetMeta()
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU0000_ARQ"])
    arq.Configure()

    sesqid = "TCPCB%04d_SESQ" % id
    sesq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[sesqid])
    tlscbid = "TlsCb%04d" % id
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_barco
    tlscb_cur.SetObjValPd()
    tlscb = copy.deepcopy(tlscb_cur)
    tcpcb = copy.deepcopy(tcb)

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(tnmdr)
    tc.pvtdata.Add(tnmpr)
    tc.pvtdata.Add(tcpcb)
    tc.pvtdata.Add(sesq)
    tc.pvtdata.Add(arq)
    return

def TestCaseVerify(tc):

    # Verify ack tx (in testspec)

    return True


def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
