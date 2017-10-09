# Testcase definition file.

import pdb
import copy
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.raw_redir_cb        import RawrCbHelper
import test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger


def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):

    tc.pvtdata = ObjectDatabase(logger)
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    RawrCbHelper.main(id)
    rawrcbid = "RawrCb%04d" % id
    # 1. Configure RAWRCB in HBM before packet injection
    rawrcb = tc.infra_data.ConfigStore.objects.db[rawrcbid]
    # let HAL fill in defaults for chain_rxq_base, etc.
    rawrcb.chain_rxq_base = 0
    rawrcb.desc_valid_bit_upd = 1
    rawrcb.desc_valid_bit_req = 0
    rawrcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    #rnmpr_small = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"])
    #rnmpr_small.Configure()
    rawrcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[rawrcbid])
    rawrcb.GetObjValPd()
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["ARQ"])
    arq.Configure()
    
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    #tc.pvtdata.Add(rnmpr_small)
    tc.pvtdata.Add(rawrcb)
    tc.pvtdata.Add(arq)
    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    rawrcbid = "RawrCb%04d" % id
    # 1. Verify chain_rxq_base
    rawrcb = tc.pvtdata.db[rawrcbid]
    rawrcb_cur = tc.infra_data.ConfigStore.objects.db[rawrcbid]
    rawrcb_cur.GetObjValPd()
    if rawrcb_cur.chain_rxq_base == 0:
        print("chain_rxq_base not set")
        return False

    print("chain_rxq_base value post-sync from HBM 0x%x" % rawrcb_cur.chain_rxq_base)

    # 2. Fetch current values from Platform
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    #rnmpr_small = tc.pvtdata.db["RNMPR_SMALL"]
    arq = tc.pvtdata.db["ARQ"]

    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    #rnmpr_small_cur = tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"]
    #rnmpr_small_cur.Configure()
    arq_cur = tc.infra_data.ConfigStore.objects.db["ARQ"]
    arq_cur.Configure()

    # 3. Verify PI for RNMDR got incremented by 1 
    if (rnmdr_cur.pi != rnmdr.pi+1):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False

    # 4. Verify PI for RNMPR or RNMPR_SMALL got incremented by 1 
    #if (rnmpr_cur.pi != rnmpr.pi+1) and (rnmpr_small_cur.pi != rnmpr_small.pi+1):
    #    print("One of RNMPR pi check failed old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
    #    print("Or RNMPR_SMALL pi check failed old %d new %d" % (rnmpr_small.pi, rnmpr_small_cur.pi))
    #    #return False

    # 5. Verify PI for ARQ got incremented by 1 
    if (arq_cur.pi != arq.pi+1):
        print("ARQ pi check failed old %d new %d" % (arq.pi, arq_cur.pi))
        #return False

    # 6. Verify descriptor
    if rnmdr.ringentries[rnmdr.pi].handle != arq_cur.ringentries[arq.pi].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdr.ringentries[rnmdr.pi].handle, arq_cur.ringentries[arq.pi].handle))
        #return False

    if rnmdr.swdre_list[rnmdr.pi].DescAddr != arq_cur.swdre_list[arq.pi].DescAddr:
        print("Descriptor handle not as expected in swdre_list 0x%x 0x%x" % (rnmdr.swdre_list[rnmdr.pi].DescAddr, arq_cur.swdre_list[arq.pi].DescAddr))
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
