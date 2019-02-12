# Testcase definition file.

import pdb
import copy

import iris.config.hal.api           as halapi
from iris.config.store               import Store
from iris.config.objects.proxycb_service    import ProxyCbServiceHelper
from iris.config.objects.cpucb        import CpuCbHelper
from infra.common.glopts import GlobalOptions

from bitstring import BitArray
import iris.test.callbacks.networking.modcbs as modcbs
import iris.test.callbacks.eth.toeplitz as toeplitz

cpurx_dpr = 0
arq = []

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    modcbs.Teardown(infra, module)
    return

def TestCaseSetup(tc):
    global cpurx_dpr
    global arq
    
    modcbs.TestCaseSetup(tc)

    id = 0 
    CpuCbHelper.main(id)
    
    # Clone objects that are needed for verification
    cpurx_dpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"])
    cpurx_dpr.GetMeta()
    cpurx_dpr.GetRingEntries([cpurx_dpr.pi])
    for i in range(3):
        arqid = ('CPU%04d_ARQ' % i)
        arq.insert(i, copy.deepcopy(tc.infra_data.ConfigStore.objects.db[arqid]))
        arq[i].GetMeta()
    
    return

def TestCaseVerify(tc):
    return True

def getCpuId(tc, step):
    if tc.config.flow.IsIPV4():
        print("IPv4 flow")
        hash_input = toeplitz.toeplitz_input(toeplitz.RSS.IPV4_TCP,
                                  step.trigger.packets[0].packet.GetScapyPacket())
    else:
        print("IPv6 flow")
        hash_input = toeplitz.toeplitz_input(toeplitz.RSS.IPV6_TCP,
                                  step.trigger.packets[0].packet.GetScapyPacket())
    print("Input for the hash:", hash_input)
    hash_value = toeplitz.toeplitz_hash(hash_input,
                                BitArray(bytes=toeplitz.toeplitz_symmetric_key))
    print("Calculated hash: 0x%x" % hash_value)
    # get the last 2 bits to get cpu-id
    cpu_id = hash_value & 0x1
    cpu_id = cpu_id if cpu_id < 1 else 0
    return cpu_id


def TestCaseStepVerify(tc, step):
    global cpurx_dpr
    global arq

    cpu_id = getCpuId(tc, step)
    print("CPU id: %d" % cpu_id)

    if GlobalOptions.dryrun:
        return True

    # 1. Fetch current values from Platform
    cpurx_dpr_cur = tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"]
    cpurx_dpr_cur.GetMeta()
    arqid = 'CPU%04d_ARQ' % cpu_id
    arq_cur = tc.infra_data.ConfigStore.objects.db[arqid]
    arq_cur.GetMeta()
    arq_cur.GetRingEntries([arq[cpu_id].pi])
    arq_cur.GetRingEntryAOL([arq[cpu_id].pi])

    # 2. Verify PI for CPU_RX_DPR got incremented by 1
    if (cpurx_dpr_cur.pi != cpurx_dpr.pi+1):
        print("CPU_RX_DPR pi check failed old %d new %d" % (cpurx_dpr.pi, cpurx_dpr_cur.pi))
        return False


    # 3. Verify PI for ARQ got incremented by 1
    print("ARQ PI: %d" % arq_cur.pi)
    if (arq_cur.pi != arq[cpu_id].pi+1):
        print(" ARQ pi check failed old %d new %d" % (arq[cpu_id].pi, arq_cur.pi))
        return False

    # 4. Verify descriptor
    # get descriptor after clearing valid bit
    descr_addr = arq_cur.ringentries[arq[cpu_id].pi].handle
    descr_addr = descr_addr & ((1 << 63) - 1)
    if cpurx_dpr.ringentries[cpurx_dpr.pi].handle != descr_addr:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (cpurx_dpr.ringentries[cpurx_dpr.pi].handle, descr_addr))
        return False

    # update all queues for the next step
    cpurx_dpr.GetMeta()
    cpurx_dpr.GetRingEntries([cpurx_dpr.pi])
    arq[cpu_id].GetMeta()
    return True

   
def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    modcbs.TestCaseTeardown(tc)
    return
