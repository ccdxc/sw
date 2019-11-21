# Testcase definition file.

import pdb
import copy
from iris.config.objects.proxycb_service    import ProxyCbServiceHelper
from iris.config.objects.raw_redir_cb        import RawrCbHelper
from iris.config.objects.raw_chain_cb        import RawcCbHelper
import iris.test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
import iris.test.app_redir.app_redir_shared as app_redir_shared

cpurx_dpr = 0
rawrcbid = ""
rawccbid = ""
rawrcb = 0
rawccb = 0
redir_span = False

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
    global rawrcbid
    global rawccbid
    global rawrcb
    global rawccb
    global redir_span
    tc.SetRetryEnabled(True)
    tc.pvtdata = ObjectDatabase()
    redir_span = getattr(tc.module.args, 'redir_span', False)
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    if redir_span:
        id = app_redir_shared.app_redir_span_rawrcb_id

    rawrcbid = "RawrCb%04d" % id
    rawccbid = "RawcCb%04d" % id

    RawrCbHelper.main(id)
    rawrcb = tc.infra_data.ConfigStore.objects.db[rawrcbid]
    RawcCbHelper.main(id)
    rawccb = tc.infra_data.ConfigStore.objects.db[rawccbid]

    # 1. Configure RAWRCB in HBM before packet injection
    # let HAL fill in defaults for chain_rxq_base, etc.
    rawrcb.chain_txq_base = 0
    rawrcb.chain_rxq_base = 0
    rawrcb.rawrcb_flags = app_redir_shared.app_redir_dol_pipeline_loopbk_en
    rawrcb.SetObjValPd()

    # 1. Configure RAWCCB in HBM before packet injection
    # let HAL fill in defaults for my_txq_base, etc.
    rawccb.my_txq_base = 0
    rawccb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    cpurx_dpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"])
    cpurx_dpr.GetMeta()
    cpurx_dpr.GetRingEntries([cpurx_dpr.pi])

    rawrcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[rawrcbid])
    rawrcb.GetObjValPd()
    rawccb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[rawccbid])
    rawccb.GetObjValPd()
    
    return

def TestCaseVerify(tc):
    global cpurx_dpr
    global rawrcbid
    global rawccbid
    global rawrcb
    global rawccb
    global redir_span

    num_pkts = 1
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = int(tc.module.args.num_pkts)

    num_flow_miss_pkts = 0
    if hasattr(tc.module.args, 'num_flow_miss_pkts'):
        num_flow_miss_pkts = int(tc.module.args.num_flow_miss_pkts)

    # Verify chain_rxq_base
    rawrcb_cur = tc.infra_data.ConfigStore.objects.db[rawrcbid]
    rawrcb_cur.GetObjValPd()
    if rawrcb_cur.chain_rxq_base == 0:
        print("chain_rxq_base not set")
        return False

    print("chain_rxq_base value post-sync from HBM 0x%x" % rawrcb_cur.chain_rxq_base)

    # Verify my_txq_base
    rawccb_cur = tc.infra_data.ConfigStore.objects.db[rawccbid]
    rawccb_cur.GetObjValPd()
    if not redir_span:
        if rawccb_cur.my_txq_base == 0:
            print("my_txq_base not set")
            return False

        print("my_txq_base value post-sync from HBM 0x%x" % rawccb_cur.my_txq_base)

    # Fetch current values from Platform
    cpurx_dpr_cur = tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"]
    cpurx_dpr_cur.GetMeta()

    # Verify PI for CPU_RX_DPR got incremented
    if (cpurx_dpr_cur.pi != cpurx_dpr.pi+num_pkts):
        print("CPU_RX_DPR pi check failed old %d new %d expected %d" %
                     (cpurx_dpr.pi, cpurx_dpr_cur.pi, cpurx_dpr.pi+num_pkts))
        rawrcb_cur.StatsPrint()
        rawccb_cur.StatsPrint()
        return False
    print("CPU_RX_DPR pi old %d new %d" % (cpurx_dpr.pi, cpurx_dpr_cur.pi))

    # Rx: verify # packets redirected
    num_redir_pkts = num_pkts - num_flow_miss_pkts
    if (rawrcb_cur.redir_pkts != rawrcb.redir_pkts+num_redir_pkts):
        print("redir_pkts check failed old %d new %d expected %d" %
              (rawrcb.redir_pkts, rawrcb_cur.redir_pkts, rawrcb.redir_pkts+num_redir_pkts))
        rawrcb_cur.StatsPrint()
        rawccb_cur.StatsPrint()
        return False
    print("redir_pkts old %d new %d" % 
          (rawrcb.redir_pkts, rawrcb_cur.redir_pkts))

    # Tx: verify PI for RAWCCB got incremented
    rawccb_cur.GetObjValPd()
    num_exp_rawccb_pkts = num_pkts
    if redir_span:
        num_exp_rawccb_pkts = 0

    if (rawccb_cur.pi != rawccb.pi+num_exp_rawccb_pkts):
        print("RAWCCB pi check failed old %d new %d expected %d" %
                      (rawccb.pi, rawccb_cur.pi, rawccb.pi+num_exp_rawccb_pkts))
        rawrcb_cur.StatsPrint()
        rawccb_cur.StatsPrint()
        return False
    print("RAWCCB pi old %d new %d" % (rawccb.pi, rawccb_cur.pi))

    # Tx: verify # packets chained
    if (rawccb_cur.chain_pkts != rawccb.chain_pkts+num_exp_rawccb_pkts):
        print("chain_pkts check failed old %d new %d expected %d" %
              (rawccb.chain_pkts, rawccb_cur.chain_pkts, rawccb.chain_pkts+num_exp_rawccb_pkts))
        rawrcb_cur.StatsPrint()
        rawccb_cur.StatsPrint()
        return False
    print("chain_pkts old %d new %d" % 
          (rawccb.chain_pkts, rawccb_cur.chain_pkts))

    rawrcb_cur.StatsPrint()
    rawccb_cur.StatsPrint()
    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    modcbs.TestCaseTeardown(tc)
    return

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def TestCaseStepVerify(tc, step):
    return modcbs.TestCaseStepVerify(tc, step)

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)

