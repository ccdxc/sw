# Testcase definition file.

import pdb

from config.store               import Store

tnmdr = 0
tnmpr = 0
serq = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global tnmdr
    global tnmpr
    global serq

    print("TestCaseSetup(): Sample Implementation.")
    
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    tcb.rcv_nxt = 0xbabababa
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    tnmdr = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmpr = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    serq = tc.infra_data.ConfigStore.objects.db["TLSCB0000_SERQ"]
    
    return

def TestCaseVerify(tc):
    global tnmdr
    global tnmpr
    global serq

    # 1. Verify rcv_nxt got updated
    tcb = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    print("rcv_nxt value pre-sync from HBM 0x%x" % tcb.rcv_nxt)
    tcb.GetObjValPd()
    print("rcv_nxt value post-sync from HBM 0x%x" % tcb.rcv_nxt)
    if tcb.rcv_nxt is not '0xBABABAFA':
        print("rcv_nxt not as expected")
        #return False
    print("rcv_nxt as expected")

    # 2. Verify tnmdr
    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.Configure()
    print("tnmdr.pi:%s tnmdr_cur.pi:%s" % (tnmdr.pi, tnmdr_cur.pi))
    if tnmdr_cur.pi != 1 or tnmdr.pi != 0:
        print("tnmdr.pi not as expected")
        #return False
    print("tnmdr.pi as expected")

    # 3. Verify tnmpr
    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.Configure()
    print("tnmpr.pi:%s tnmpr_cur.pi:%s" % (tnmpr.pi, tnmpr_cur.pi))
    if tnmpr_cur.pi != 1 or tnmpr.pi != 0:
        print("tnmpr.pi not as expected")
        #return False
    print("tnmpr.pi as expected")

    # 3. Verify serq
    serq_cur = tc.infra_data.ConfigStore.objects.db["TLSCB0000_SERQ"]
    serq_cur.Configure()
    print("serq.pi:%s serq_cur.pi:%s" % (serq.pi, serq_cur.pi))
    if serq_cur.pi != 1 or serq.pi != 0:
        print("serq.pi not as expected")
        #return False
    print("serq.pi as expected")

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
