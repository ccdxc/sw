# Testcase definition file.

import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper

rnmdr = 0
rnmpr = 0
serq = 0
tlscb = 0 
tcpcb = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    elem = module.iterator.Get()
    module.testspec.selectors.flow.Extend(elem.flow)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global serq
    global tlscb 
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
    tcb.debug_dol = tcp_proxy.tcp_debug_dol_pkt_to_serq + \
                    tcp_proxy.tcp_debug_dol_test_atomic_stats
    print("debug_dol = %d" % tcb.debug_dol)
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    serqid = "TLSCB%04d_SERQ" % id
    serq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[serqid])
    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])
    tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid])
    
    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global serq
    global tlscb 
    global tcpcb

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tcbid = "TcpCb%04d" % id
    # 1. Verify rcv_nxt got updated
    tcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    print("rcv_nxt value pre-sync from HBM 0x%x" % tcb_cur.rcv_nxt)
    tcb_cur.GetObjValPd()
    print("rcv_nxt value post-sync from HBM 0x%x" % tcb_cur.rcv_nxt)
    if tcb_cur.rcv_nxt != 0xbababb0e:
        print("rcv_nxt not as expected")
        return False
    print("rcv_nxt as expected")

    # 2. Verify pi/ci got update got updated
    tlscbid = "TlsCb%04d" % id
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    print("pre-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    tlscb_cur.GetObjValPd()
    print("post-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    if (tlscb_cur.serq_pi != tlscb.serq_pi or tlscb_cur.serq_ci != tlscb.serq_ci):
        print("serq pi/ci not as expected")
        return False

    # 3. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    serqid = "TLSCB%04d_SERQ" % id
    serq_cur = tc.infra_data.ConfigStore.objects.db[serqid]
    serq_cur.Configure()

    # 4. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False

    # 5. Verify descriptor 
    if rnmdr.ringentries[rnmdr.pi].handle != serq_cur.ringentries[tlscb.serq_pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, serq_cur.ringentries[tlscb.serq_pi].handle)) 
        return False

    if rnmdr.swdre_list[rnmdr.pi].DescAddr != serq_cur.swdre_list[tlscb.serq_pi].DescAddr:
        print("Descriptor handle not as expected in swdre_list 0x%x 0x%x" % (rnmdr.swdre_list[rnmdr.pi].DescAddr, serq_cur.swdre_list[tlscb.serq_pi].DescAddr))
        return False

    # 6. Verify page
    if rnmpr.ringentries[0].handle != serq_cur.swdre_list[0].Addr1:
        print("Page handle not as expected in serq_cur.swdre_list")
        #return False

    # Print stats
    print("bytes_rcvd = %d:" % tcb_cur.bytes_rcvd)
    print("pkts_rcvd = %d:" % tcb_cur.pkts_rcvd)
    print("pages_alloced = %d:" % tcb_cur.pages_alloced)
    print("desc_alloced = %d:" % tcb_cur.desc_alloced)

    #7 Verify pkt stats
    if tcb_cur.pkts_rcvd != tcpcb.pkts_rcvd + 1:
        print("pkt rx stats not as expected")
        return False

    if tcb_cur.bytes_rcvd != tcpcb.bytes_rcvd + 84:
        print("Warning! pkt rx byte stats not as expected")

    #8 Verify page stats
    if tcb_cur.pages_alloced != tcpcb.pages_alloced + 1:
        print("pages alloced stats not as expected")
        return False
    
    #9 Verify descr stats
    if tcb_cur.desc_alloced != tcpcb.desc_alloced + 1:
        print("desc alloced stats not as expected")
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
