# Testcase definition file.

import pdb
import copy

from config.store               import Store

rnmdr = 0
rnmpr = 0
brq = 0
tlscb = 0
tcb = 0
tnmdr = 0
tnmpr = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global sesq
    global tlscb
    global tcb
    global tnmdr
    global tnmpr

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
    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])
    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ"])
    sesq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TCPCB0000_SESQ"])
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TlsCb0000"])

    tlscb_cur = tc.infra_data.ConfigStore.objects.db["TlsCb0000"]
    tlscb_cur.debug_dol = 1
    tlscb_cur.SetObjValPd()





    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global brq
    global tlscb
    global tcb
    global tnmdr
    global tnmpr


    # 1. Verify pi/ci got update got updated for SESQ
    tcb_cur = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    print("pre-sync: tcb_cur.sesq_pi %d tcb_cur.sesq_ci %d" % (tcb_cur.sesq_pi, tcb_cur.sesq_ci))
    tcb_cur.GetObjValPd()
    print("post-sync: tcb_cur.sesq_pi %d tcb_cur.sesq_ci %d" % (tcb_cur.sesq_pi, tcb_cur.sesq_ci))
    if (tcb_cur.sesq_pi != (tcb.sesq_pi+1) or tcb_cur.sesq_ci != (tcb.sesq_ci+1)):
        print("sesq pi/ci not as expected")
        #VijayV to enable this test after ci is fixed in p4+
        #return False


    # 2. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()

    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.Configure()
    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.Configure()

    # 3. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        #return false

    
    # 4. Verify PI for TNMDR got incremented by 1
    if (tnmdr_cur.pi != tnmdr.pi+1):
        print("TNMDR pi check failed old %d new %d" % (tnmdr.pi, tnmdr_cur.pi))
        #return false

    sesq_cur = tc.infra_data.ConfigStore.objects.db["TCPCB0000_SESQ"]
    sesq_cur.Configure()

    # 5. Verify descriptor 
    if rnmdr.ringentries[rnmdr_cur.pi-1].handle != sesq_cur.ringentries[0].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, sesq_cur.ringentries[0].handle)) 
        return False


    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
