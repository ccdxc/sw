# Testcase definition file.

import pdb
import copy

from iris.config.store               import Store
from iris.config.objects.proxycb_service    import ProxyCbServiceHelper
from iris.config.objects.cpucb        import CpuCbHelper
from infra.common.glopts import GlobalOptions

cpurx_dpr = 0
arq = 0

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    global cpurx_dpr
    global arq

    id = 0 
    CpuCbHelper.main(id)
    # Clone objects that are needed for verification
    cpurx_dpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"])
    cpurx_dpr.GetMeta()
    cpurx_dpr.GetRingEntries([cpurx_dpr.pi])
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU0000_ARQ"])
    arq.GetMeta()
    
    return

def TestCaseVerify(tc):
    global cpurx_dpr
    global arq

    if GlobalOptions.dryrun:
        return True

    # 1. Fetch current values from Platform
    cpurx_dpr_cur = tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"]
    cpurx_dpr_cur.GetMeta()
    arq_cur = tc.infra_data.ConfigStore.objects.db["CPU0000_ARQ"]
    arq_cur.GetMeta()
    arq_cur.GetRingEntries([arq.pi])
    arq_cur.GetRingEntryAOL([arq.pi])

    # 2. Verify PI for CPU_RX_DPR got incremented by 1
    if (cpurx_dpr_cur.pi != cpurx_dpr.pi+1):
        print("CPU_RX_DPR pi check failed old %d new %d" % (cpurx_dpr.pi, cpurx_dpr_cur.pi))
        return False

    # 3. Verify PI for ARQ got incremented by 1
    if (arq_cur.pi != arq.pi+1):
        print("CPU0000_ARQ pi check failed old %d new %d" % (arq.pi, arq_cur.pi))
        return False

    # 4. Verify descriptor
    #if cpurx_dpr.ringentries[cpurx_dpr.pi].handle != arq_cur.ringentries[arq.pi].handle:
    if cpurx_dpr.ringentries[cpurx_dpr.pi].handle != arq_cur.ringentries[arq.pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (cpurx_dpr.ringentries[cpurx_dpr.pi].handle, arq_cur.ringentries[arq.pi].handle))
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
