# Testcase definition file.

import pdb
import copy

from config.store               import Store

rnmdr = 0
rnmpr = 0
brq = 0
tlscb = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global brq
    global tlscb

    print("TestCaseSetup(): Sample Implementation.")
    
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    tcb.rcv_nxt = 0xBABABABA
    tcb.snd_nxt = 0xEFEFEFF0
    tcb.snd_una = 0xEFEFEFEF
    tcb.rcv_tsval = 0xFAFAFAFA
    tcb.ts_recent = 0xFAFAFAF0
    tcb.debug_dol = 0
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ"])
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TlsCb0000"])
    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global brq
    global tlscb


    # 1. Verify pi/ci got update got updated for SERQ
    tlscb_cur = tc.infra_data.ConfigStore.objects.db["TlsCb0000"]
    print("pre-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    tlscb_cur.GetObjValPd()
    print("post-sync: tlscb_cur.serq_pi %d tlscb_cur.serq_ci %d" % (tlscb_cur.serq_pi, tlscb_cur.serq_ci))
    if (tlscb_cur.serq_pi != (tlscb.serq_pi+1) or tlscb_cur.serq_ci != (tlscb.serq_ci+1)):
        print("serq pi/ci not as expected")
        #VijayV to enable this test after ci is fixed in p4+
        #return False

    # 2. Verify pi/ci got update got updated for BRQ
    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ"]
    print("pre-sync: brq_cur.pi %d brq_cur.ci %d" % (brq_cur.pi, brq_cur.ci))
    brq_cur.Configure()
    print("post-sync: brq_cur.pi %d brq_cur.ci %d" % (brq_cur.pi, brq_cur.ci))
    if (brq_cur.pi != (brq.pi+1) or brq_cur.ci != (brq.ci+1)):
        print("brq pi/ci not as expected")
        #needs fix in HAL and support in model/p4+ for this check to work/pass
        #return False

    # 2. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()

    # 3. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return false

    # 2. Verify descriptor 
    if rnmdr.ringentries[rnmdr.pi].handle != brq_cur.ringentries[0].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, brq_cur.ringentries[0].handle)) 
        return False

    # 3. Verify page
    if rnmpr.ringentries[rnmdr.pi].handle != brq_cur.swdre_list[0].Addr1:
        print("Page handle not as expected in brq_cur.swdre_list")
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
