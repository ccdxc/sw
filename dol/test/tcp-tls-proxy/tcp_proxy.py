# Testcase definition file.

import pdb

from config.store               import Store

rnmdr = 0
rnmpr = 0
serq = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global serq

    print("TestCaseSetup(): Sample Implementation.")
    
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    tcb.rcv_nxt = 0xBABABABA
    tcb.snd_nxt = 0xEFEFEFEF
    tcb.snd_una = 0xEFEFEFEF
    tcb.rcv_tsval = 0xFAFAFAFA
    tcb.ts_recent = 0xFAFAFAF0
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmpr = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    serq = tc.infra_data.ConfigStore.objects.db["TLSCB0000_SERQ"]
    
    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global serq

    # 1. Verify rcv_nxt got updated
    tcb = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    print("rcv_nxt value pre-sync from HBM 0x%x" % tcb.rcv_nxt)
    tcb.GetObjValPd()
    print("rcv_nxt value post-sync from HBM 0x%x" % tcb.rcv_nxt)
    if tcb.rcv_nxt is not '0xBABABAFA':
        print("rcv_nxt not as expected")
        return False
    print("rcv_nxt as expected")

    # 2. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    serq_cur = tc.infra_data.ConfigStore.objects.db["TLSCB0000_SERQ"]
    serq_cur.Configure()

    # 3. Verify descriptor 
    if rnmdr.ringentries[0].handle != serq_cur.ringentries[0].handle:
        print("Descriptor handle not as expected in ringentries") 
        return False

    if rnmdr.swdre_list[0].DescAddr != serq_cur.swdre_list[0].DescAddr:
        print("Descriptor handle not as expected in swdre_list")
        return False

    # 4. Verify page
    if rnmpr.ringentries[0].handle != serq_cur.swdre_list[0].Addr1:
        print("Page handle not as expected in serq_cur.swdre_list")
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
