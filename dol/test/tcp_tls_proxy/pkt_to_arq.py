# Testcase definition file.

import pdb
import copy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper

rnmdr = 0
rnmpr = 0
arq = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global arq

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
    tcb.debug_dol = 0
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["ARQ"])
    arq.Configure()
    
    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global arq

    # 1. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    arq_cur = tc.infra_data.ConfigStore.objects.db["ARQ"]
    arq_cur.Configure()

    # 2. Verify PI for RNMDR got incremented by 2 
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        #return False

    # 3. Verify PI for ARQ got incremented by 1
    if (arq_cur.pi != arq.pi+1):
        print("ARQ pi check failed old %d new %d" % (arq.pi, arq_cur.pi))
        #return False

    # 2. Verify descriptor
    #if rnmdr.ringentries[rnmdr.pi].handle != arq_cur.ringentries[arq.pi].handle:
    if rnmdr.ringentries[rnmdr.pi].handle != arq_cur.ringentries[0].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, arq_cur.ringentries[0].handle))
        #print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, arq_cur.ringentries[arq.pi].handle))
        #return False

    print("Descriptor handle as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, arq_cur.ringentries[0].handle))
    # 3. Verify page
    if rnmpr.ringentries[0].handle != arq_cur.swdre_list[0].Addr1:
        print("Page handle not as expected in arq_cur.swdre_list")
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
