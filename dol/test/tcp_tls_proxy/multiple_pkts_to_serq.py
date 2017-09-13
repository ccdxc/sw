# Testcase definition file.

import pdb
import copy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper

rnmdr = 0
rnmpr = 0
serq = 0
tlscb = 0 

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
    tcb.debug_dol = 1
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    serqid = "TLSCB%04d_SERQ" % id
    serq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[serqid])
    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])
    
    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global serq
    global tlscb 

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tcbid = "TcpCb%04d" % id
    # 1. Verify rcv_nxt got updated
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    print("rcv_nxt value pre-sync from HBM 0x%x" % tcb.rcv_nxt)
    tcb.GetObjValPd()
    print("rcv_nxt value post-sync from HBM 0x%x" % tcb.rcv_nxt)
    if tcb.rcv_nxt != 0x62bbbaba:
        print("rcv_nxt not as expected")
        return False
    print("rcv_nxt as expected")


    # 3. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()

    # 4. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+2):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return false

    # 5. Print stats
    print("bytes_rcvd = %d:" % tcb.bytes_rcvd)
    print("pkts_rcvd = %d:" % tcb.pkts_rcvd)
    print("pages_alloced = %d:" % tcb.pages_alloced)
    print("desc_alloced = %d:" % tcb.desc_alloced)

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
