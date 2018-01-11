# Testcase definition file.

import pdb
import copy

from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.cpucb        import CpuCbHelper
from infra.common.glopts import GlobalOptions

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

    id = 0 
    CpuCbHelper.main(id)
    cpucbid = "CpuCb%04d" % id
    # 1. Configure CPUCB in HBM before packet injection
    cpucb = tc.infra_data.ConfigStore.objects.db[cpucbid]
    cpucb.debug_dol = 1
    cpucb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.GetMeta()
    rnmdr.GetRingEntries([rnmdr.pi])
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.GetMeta()
    rnmpr.GetRingEntries([rnmpr.pi])
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU0000_ARQ"])
    arq.GetMeta()
    
    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global arq

    if GlobalOptions.dryrun:
        return True

    # 1. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.GetMeta()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.GetMeta()
    arq_cur = tc.infra_data.ConfigStore.objects.db["CPU0000_ARQ"]
    arq_cur.GetMeta()
    arq_cur.GetRingEntries([arq.pi])
    arq_cur.GetRingEntryAOL([arq.pi])

    # 2. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False

    # 3. Verify PI for ARQ got incremented by 1
    if (arq_cur.pi != arq.pi+1):
        print("CPU0000_ARQ pi check failed old %d new %d" % (arq.pi, arq_cur.pi))
        return False

    # 4. Verify descriptor
    #if rnmdr.ringentries[rnmdr.pi].handle != arq_cur.ringentries[arq.pi].handle:
    if rnmdr.ringentries[rnmdr.pi].handle != arq_cur.ringentries[arq.pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, arq_cur.ringentries[0].handle))
        return False

    # 5. Verify PI for RNMPR got incremented by 1
    if (rnmpr_cur.pi != rnmpr.pi+1):
        print("RNMPR pi check failed old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
        return False

    # 6. Verify page
    if rnmpr.ringentries[rnmpr.pi].handle != arq_cur.swdre_list[0].Addr1:
        print("Page handle not as expected in arq_cur.swdre_list 0x%x 0x%x" %(rnmpr.ringentries[0].handle, arq_cur.swdre_list[0].Addr1))
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    id = 0 
    CpuCbHelper.main(id)
    cpucbid = "CpuCb%04d" % id
    # 1. Configure CPUCB in HBM before packet injection
    cpucb = tc.infra_data.ConfigStore.objects.db[cpucbid]
    cpucb.debug_dol = 0
    cpucb.SetObjValPd()
    return
