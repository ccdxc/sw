# Testcase definition file.

import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
import test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger


def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):

    tc.pvtdata = ObjectDatabase(logger)
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcp_proxy.init_tcb_inorder(tc, tcb)
    tcb.SetObjValPd()

    TcpCbHelper.main(id + 1)
    tcbid2 = "TcpCb%04d" % (id + 1)
    tcb2 = tc.infra_data.ConfigStore.objects.db[tcbid2]
    tcp_proxy.init_tcb_inorder(tc, tcb2)
    tcb2.SetObjValPd()

    # 2. Configure TLS CB in HBM before packet injection
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb.debug_dol = 1
    tlscb.SetObjValPd()

    tlscbid2 = "TlsCb%04d" % (id + 1)
    tlscb2 = tc.infra_data.ConfigStore.objects.db[tlscbid2]
    tlscb2.debug_dol = 1
    tlscb2.SetObjValPd()

    # 3. Clone objects that are needed for verification
    tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid])
    tcpcb.GetObjValPd()
    tc.pvtdata.Add(tcpcb)
    tcpcb2 = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid2])
    tc.pvtdata.Add(tcpcb2)
    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    if tc.config.flow.IsIflow():
        print("This is iflow")
        tcbid = "TcpCb%04d" % id
    else:
        print("This is rflow")
        tcbid = "TcpCb%04d" % (id + 1)
    tcpcb = tc.pvtdata.db[tcbid]
    tcpcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcpcb_cur.GetObjValPd()

    # Print stats
    print("bytes_sent = %d:" % tcpcb_cur.bytes_sent)
    print("pkts_sent = %d:" % tcpcb_cur.pkts_sent)
    print("debug_num_phv_to_pkt = %d:" % tcpcb_cur.debug_num_phv_to_pkt)
    print("debug_num_mem_to_pkt = %d:" % tcpcb_cur.debug_num_mem_to_pkt)

    # 1. Verify pi got updated
    if (tcpcb_cur.sesq_pi != tcpcb.sesq_pi + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tcpcb.sesq_pi, tcpcb.sesq_ci,
                 tcpcb_cur.sesq_pi, tcpcb_cur.sesq_ci))
        return False

    # 2. Verify ci got updated
    if (tcpcb_cur.sesq_ci != tcpcb.sesq_ci + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tcpcb.sesq_pi, tcpcb.sesq_ci,
                 tcpcb_cur.sesq_pi, tcpcb_cur.sesq_ci))
        return False

    # 3. Verify pkt stats
    if tcpcb_cur.pkts_sent != tcpcb.pkts_sent + 1:
        print("pkt tx stats not as expected")
        return False

    if ((tcpcb_cur.bytes_sent - tcpcb.bytes_sent) != 54):
        print("Warning! pkt tx byte stats not as expected %d %d" % (tcpcb_cur.bytes_sent, tcpcb.bytes_sent))
        return False
    
    # 4. Verify phv2pkt
    if tcpcb_cur.debug_num_phv_to_pkt != tcpcb.debug_num_phv_to_pkt + 2:
        print("Num phv2pkt not as expected")
        return False

    # 5. Verify mem2pkt
    if tcpcb_cur.debug_num_mem_to_pkt != tcpcb.debug_num_mem_to_pkt + 2:
        print("Num mem2pkt not as expected")
        return False

    # 6  Verify phv2mem
    if tcpcb_cur.snd_nxt != 0xefeff044:
        print("mem2pkt failed snd_nxt = 0x%x" % tcpcb_cur.snd_nxt)
        return False

    # 7. Verify pkt tx (in testspec)

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
