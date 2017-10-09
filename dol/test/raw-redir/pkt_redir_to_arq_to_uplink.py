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
    rawrcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[rawrcbid])
    rawrcb.GetObjValPd()
    
    tc.pvtdata.Add(rawrcb)
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

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
