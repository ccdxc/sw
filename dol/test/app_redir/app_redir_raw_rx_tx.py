# Testcase definition file.

import pdb
import copy
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.raw_redir_cb        import RawrCbHelper
from config.objects.raw_chain_cb        import RawcCbHelper
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
    rawrcb.SetObjValPd()

    RawcCbHelper.main(id)
    rawccbid = "RawcCb%04d" % id
    # 1. Configure RAWCCB in HBM before packet injection
    rawccb = tc.infra_data.ConfigStore.objects.db[rawccbid]
    # let HAL fill in defaults for my_txq_base, etc.
    rawccb.my_txq_base = 0
    rawccb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    rnmpr_small = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"])
    rnmpr_small.Configure()
    rawrcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[rawrcbid])
    rawrcb.GetObjValPd()
    rawccb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[rawccbid])
    rawccb.GetObjValPd()
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["ARQ"])
    arq.Configure()
    rawccbq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RAWCCBQ"])
    rawccbq.Configure()
    
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(rnmpr_small)
    tc.pvtdata.Add(arq)
    tc.pvtdata.Add(rawrcb)
    tc.pvtdata.Add(rawccb)
    tc.pvtdata.Add(rawccbq)
    return

def TestCaseVerify(tc):

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    # Verify chain_rxq_base
    rawrcbid = "RawrCb%04d" % id
    rawrcb = tc.pvtdata.db[rawrcbid]
    rawrcb_cur = tc.infra_data.ConfigStore.objects.db[rawrcbid]
    rawrcb_cur.GetObjValPd()
    if rawrcb_cur.chain_rxq_base == 0:
        print("chain_rxq_base not set")
        return False

    print("chain_rxq_base value post-sync from HBM 0x%x" % rawrcb_cur.chain_rxq_base)

    # Verify my_txq_base
    rawccbid = "RawcCb%04d" % id
    rawccb = tc.pvtdata.db[rawccbid]
    rawccb_cur = tc.infra_data.ConfigStore.objects.db[rawccbid]
    rawccb_cur.GetObjValPd()
    if rawccb_cur.my_txq_base == 0:
        print("my_txq_base not set")
        return False

    print("my_txq_base value post-sync from HBM 0x%x" % rawccb_cur.my_txq_base)

    # Fetch current values from Platform
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    rnmpr_small = tc.pvtdata.db["RNMPR_SMALL"]
    arq = tc.pvtdata.db["ARQ"]
    rawccbq = tc.pvtdata.db["RAWCCBQ"]

    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    rnmpr_small_cur = tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"]
    rnmpr_small_cur.Configure()
    arq_cur = tc.infra_data.ConfigStore.objects.db["ARQ"]
    arq_cur.Configure()
    rawccbq_cur = tc.infra_data.ConfigStore.objects.db["RAWCCBQ"]
    rawccbq_cur.Configure()

    # Verify PI for RNMDR got incremented
    if (rnmdr_cur.pi == rnmdr.pi):
        print("RNMDR pi check failed old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))
        return False
    print("RNMDR pi old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))

    # Verify PI for RNMPR or RNMPR_SMALL got incremented
    if (rnmpr_cur.pi == rnmpr.pi) and (rnmpr_small_cur.pi == rnmpr_small.pi):
        print("One of RNMPR pi check failed old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
        print("Or RNMPR_SMALL pi check failed old %d new %d" % (rnmpr_small.pi, rnmpr_small_cur.pi))
        return False
    print("RNMPR pi old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
    print("RNMPR_SMALL old %d new %d" % (rnmpr_small.pi, rnmpr_small_cur.pi))

    # Rx: verify PI for ARQ got incremented
    if (arq_cur.pi == arq.pi):
        print("ARQ pi check failed old %d new %d" % (arq.pi, arq_cur.pi))
        return False
    print("ARQ pi old %d new %d" % (arq.pi, arq_cur.pi))

    # Tx: verify PI for RAWCCB got incremented
    if (rawccb_cur.pi == rawccb.pi):
        print("RAWCCB pi check failed old %d new %d" % (rawccb.pi, rawccb_cur.pi))
        return False
    print("RAWCCB pi old %d new %d" % (rawccb.pi, rawccb_cur.pi))

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
