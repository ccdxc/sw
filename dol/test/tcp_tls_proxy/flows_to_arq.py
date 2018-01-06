# Testcase definition file.

import pdb
import copy

import config.hal.api           as halapi
from config.store               import Store
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.cpucb        import CpuCbHelper
from infra.common.glopts import GlobalOptions

from bitstring import BitArray
import test.callbacks.networking.modcbs as modcbs
import test.callbacks.eth.toeplitz as toeplitz

rnmdr = 0
rnmpr = 0
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
    global rnmdr
    global rnmpr
    global arq
    
    modcbs.TestCaseSetup(tc)

    id = 0 
    CpuCbHelper.main(id)
    cpucbid = "CpuCb%04d" % id
    # 1. Configure CPUCB in HBM before packet injection
    cpucb = tc.infra_data.ConfigStore.objects.db[cpucbid]
    cpucb.debug_dol = 1
    cpucb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    for i in range(3):
        arqid = ('CPU%04d_ARQ' % i)
        arq.insert(i, copy.deepcopy(tc.infra_data.ConfigStore.objects.db[arqid]))
        arq[i].Configure()
    
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
    cpu_id = hash_value & 0x3
    cpu_id = cpu_id if cpu_id < 3 else 0
    return cpu_id


def TestCaseStepVerify(tc, step):
    global rnmdr
    global rnmpr
    global arq

    cpu_id = getCpuId(tc, step)
    print("CPU id: %d" % cpu_id)

    if GlobalOptions.dryrun:
        return True

    # 1. Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    arqid = 'CPU%04d_ARQ' % cpu_id
    arq_cur = tc.infra_data.ConfigStore.objects.db[arqid]
    arq_cur.Configure()

    # 2. Verify PI for RNMDR got incremented by 1
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False


    # 3. Verify PI for ARQ got incremented by 1
    print("ARQ PI: %d" % arq_cur.pi)
    if (arq_cur.pi != arq[cpu_id].pi+1):
        print(" ARQ pi check failed old %d new %d" % (arq[cpu_id].pi, arq_cur.pi))
        return False

    # 4. Verify descriptor
    if rnmdr.ringentries[rnmdr.pi].handle != arq_cur.ringentries[arq[cpu_id].pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, arq_cur.ringentries[arq[cpu_id].pi].handle))
        return False

    # 5. Verify PI for RNMPR got incremented by 1
    if (rnmpr_cur.pi != rnmpr.pi+1):
        print("RNMPR pi check failed old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
        return False

    # 6. Verify page
    if rnmpr.ringentries[rnmpr.pi].handle != arq_cur.swdre_list[arq[cpu_id].pi].Addr1:
        print("Page handle not as expected in arq_cur.swdre_list 0x%x 0x%x" %(rnmpr.ringentries[rnmpr.pi].handle, arq_cur.swdre_list[arq[cpu_id].pi].Addr1))
        return False

    # update all queues for the next step
    rnmdr.Configure()
    rnmpr.Configure()
    arq[cpu_id].Configure()
    return True

   
def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    modcbs.TestCaseTeardown(tc)
    id = 0 
    CpuCbHelper.main(id)
    cpucbid = "CpuCb%04d" % id
    # 1. Configure CPUCB in HBM before packet injection
    cpucb = tc.infra_data.ConfigStore.objects.db[cpucbid]
    cpucb.debug_dol = 0
    cpucb.SetObjValPd()
    return
