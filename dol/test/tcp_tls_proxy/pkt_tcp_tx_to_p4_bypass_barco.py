# Testcase definition file.

import pdb
import copy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper

tcpcb = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global tcpcb

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcb.rcv_nxt = 0xBABABABA
    tcb.snd_nxt = 0xEFEFEFF0
    tcb.snd_una = 0xEFEFEFEF
    tcb.rcv_tsval = 0xFAFAFAFA
    tcb.ts_recent = 0xFAFAFAF0
    tcb.snd_wnd = 1000
    tcb.snd_cwnd = 1000
    tcb.rcv_mss = 9216
    tcb.debug_dol = 0
    tcb.SetObjValPd()

    # 2. Configure TLS CB in HBM before packet injection
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb.debug_dol = 1
    tlscb.SetObjValPd()

    # 3. Clone objects that are needed for verification
    tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid])
    return

def TestCaseVerify(tc):
    global tcpcb

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tcbid = "TcpCb%04d" % id
    tcpcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcpcb_cur.GetObjValPd()

    # Print stats
    print("bytes_sent = %d:" % tcpcb_cur.bytes_sent)
    print("pkts_sent = %d:" % tcpcb_cur.pkts_sent)
    print("debug_num_phv_to_pkt = %d:" % tcpcb_cur.debug_num_phv_to_pkt)
    print("debug_num_mem_to_pkt = %d:" % tcpcb_cur.debug_num_mem_to_pkt)

    # 1. Verify pi/ci got update got updated
    if (tcpcb_cur.sesq_pi != tcpcb.sesq_pi + 1 or tcpcb_cur.sesq_ci != tcpcb.sesq_ci + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tcpcb.sesq_pi, tcpcb.sesq_ci,
                 tcpcb_cur.sesq_pi, tcpcb_cur.sesq_ci))
        return False

    # 2. Verify pkt stats
    if tcpcb_cur.pkts_sent != tcpcb.pkts_sent + 1:
        print("pkt tx stats not as expected")
        return False

    if tcpcb_cur.bytes_sent != tcpcb.bytes_sent + 84:
        print("Warning! pkt tx byte stats not as expected")
    
    # 3. Verify phv2pkt
    if tcpcb_cur.debug_num_phv_to_pkt != tcpcb.debug_num_phv_to_pkt + 2:
        print("Num phv2pkt not as expected")
        return False

    # 4. Verify mem2pkt
    if tcpcb_cur.debug_num_mem_to_pkt != tcpcb.debug_num_mem_to_pkt + 2:
        print("Num mem2pkt not as expected")
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
