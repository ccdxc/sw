# Testcase definition file.

import pdb
import copy

from iris.config.store               import Store
from iris.config.objects.proxycb_service    import ProxyCbServiceHelper
from iris.config.objects.cpucb        import CpuCbHelper
from infra.common.glopts import GlobalOptions

rnmdpr_big = 0
arq = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global rnmdpr_big
    global arq

    id = 0 
    CpuCbHelper.main(id)
    # Clone objects that are needed for verification
    rnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"])
    rnmdpr_big.GetMeta()
    rnmdpr_big.GetRingEntries([rnmdpr_big.pi])
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU0000_ARQ"])
    arq.GetMeta()
    
    return

def TestCaseVerify(tc):
    global rnmdpr_big
    global arq

    if GlobalOptions.dryrun:
        return True

    # 1. Fetch current values from Platform
    rnmdpr_big_cur = tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"]
    rnmdpr_big_cur.GetMeta()
    arq_cur = tc.infra_data.ConfigStore.objects.db["CPU0000_ARQ"]
    arq_cur.GetMeta()
    arq_cur.GetRingEntries([arq.pi])
    arq_cur.GetRingEntryAOL([arq.pi])

    # 2. Verify PI for RNMDPR_BIG got incremented by 1
    if (rnmdpr_big_cur.pi != rnmdpr_big.pi+1):
        print("RNMDPR_BIG pi check failed old %d new %d" % (rnmdpr_big.pi, rnmdpr_big_cur.pi))
        return False

    # 3. Verify PI for ARQ got incremented by 1
    if (arq_cur.pi != arq.pi+1):
        print("CPU0000_ARQ pi check failed old %d new %d" % (arq.pi, arq_cur.pi))
        return False

    # 4. Verify descriptor
    #if rnmdpr.ringentries[rnmdpr.pi].handle != arq_cur.ringentries[arq.pi].handle:
    if rnmdpr_big.ringentries[rnmdpr_big.pi].handle != arq_cur.ringentries[arq.pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdpr_big.ringentries[rnmdpr_big.pi].handle, arq_cur.ringentries[arq.pi].handle))
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
