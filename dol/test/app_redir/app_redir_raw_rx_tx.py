# Testcase definition file.

import pdb
import copy
from config.objects.proxycb_service    import ProxyCbServiceHelper
from config.objects.raw_redir_cb        import RawrCbHelper
from config.objects.raw_chain_cb        import RawcCbHelper
import test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
import test.app_redir.app_redir_shared as app_redir_shared

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    modcbs.Teardown(infra, module)
    return

def TestCaseSetup(tc):

    tc.pvtdata = ObjectDatabase(logger)
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    if hasattr(tc.module.args, 'redir_span'):
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
    rawccbq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RAWCCBQ"])
    rawccbq.Configure()
    
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(rnmpr_small)
    tc.pvtdata.Add(rawrcb)
    tc.pvtdata.Add(rawccb)
    tc.pvtdata.Add(rawccbq)
    return

def TestCaseVerify(tc):

    num_pkts = 1
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = int(tc.module.args.num_pkts)

    num_flow_miss_pkts = 0
    if hasattr(tc.module.args, 'num_flow_miss_pkts'):
        num_flow_miss_pkts = int(tc.module.args.num_flow_miss_pkts)

    redir_span = False
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    if hasattr(tc.module.args, 'redir_span'):
        id = app_redir_shared.app_redir_span_rawrcb_id
        redir_span = True

    rawrcbid = "RawrCb%04d" % id
    rawccbid = "RawcCb%04d" % id

    # Verify chain_rxq_base
    rawrcb = tc.pvtdata.db[rawrcbid]
    rawrcb_cur = tc.infra_data.ConfigStore.objects.db[rawrcbid]
    rawrcb_cur.GetObjValPd()
    if rawrcb_cur.chain_rxq_base == 0:
        print("chain_rxq_base not set")
        return False

    print("chain_rxq_base value post-sync from HBM 0x%x" % rawrcb_cur.chain_rxq_base)

    # Verify my_txq_base
    rawccb = tc.pvtdata.db[rawccbid]
    rawccb_cur = tc.infra_data.ConfigStore.objects.db[rawccbid]
    rawccb_cur.GetObjValPd()
    if not redir_span:
        if rawccb_cur.my_txq_base == 0:
            print("my_txq_base not set")
            return False

        print("my_txq_base value post-sync from HBM 0x%x" % rawccb_cur.my_txq_base)

    # Fetch current values from Platform
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    rnmpr_small = tc.pvtdata.db["RNMPR_SMALL"]
    rawccbq = tc.pvtdata.db["RAWCCBQ"]

    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    rnmpr_small_cur = tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"]
    rnmpr_small_cur.Configure()
    rawccbq_cur = tc.infra_data.ConfigStore.objects.db["RAWCCBQ"]
    rawccbq_cur.Configure()

    # Verify PI for RNMDR got incremented
    if (rnmdr_cur.pi != rnmdr.pi+num_pkts):
        print("RNMDR pi check failed old %d new %d expected %d" %
                     (rnmdr.pi, rnmdr_cur.pi, rnmdr.pi+num_pkts))
        app_redir_shared.rawrcb_stats_print(tc, rawrcb_cur)
        return False
    print("RNMDR pi old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))

    # Verify PI for RNMPR or RNMPR_SMALL got incremented
    if ((rnmpr_cur.pi+rnmpr_small_cur.pi) != (rnmpr.pi+rnmpr_small.pi+num_pkts)):
        print("RNMPR pi check failed old %d new %d expected %d" %
                  (rnmpr.pi+rnmpr_small.pi, rnmpr_cur.pi+rnmpr_small_cur.pi,
                   rnmpr.pi+rnmpr_small.pi+num_pkts))
        app_redir_shared.rawrcb_stats_print(tc, rawrcb_cur)
        return False
    print("RNMPR pi old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
    print("RNMPR_SMALL old %d new %d" % (rnmpr_small.pi, rnmpr_small_cur.pi))

    # Rx: verify # packets redirected
    num_redir_pkts = num_pkts - num_flow_miss_pkts
    if (rawrcb_cur.stat_pkts_redir != rawrcb.stat_pkts_redir+num_redir_pkts):
        print("stat_pkts_redir check failed old %d new %d expected %d" %
              (rawrcb.stat_pkts_redir, rawrcb_cur.stat_pkts_redir, rawrcb.stat_pkts_redir+num_redir_pkts))
        app_redir_shared.rawrcb_stats_print(tc, rawrcb_cur)
        return False
    print("stat_pkts_redir old %d new %d" % 
          (rawrcb.stat_pkts_redir, rawrcb_cur.stat_pkts_redir))

    # Tx: verify PI for RAWCCB got incremented
    rawccb_cur.GetObjValPd()
    if not redir_span:
        if (rawccb_cur.pi != rawccb.pi+num_pkts):
            print("RAWCCB pi check failed old %d new %d expected %d" %
                          (rawccb.pi, rawccb_cur.pi, rawccb.pi+num_pkts))
            app_redir_shared.rawccb_stats_print(tc, rawccb_cur)
            return False
        print("RAWCCB pi old %d new %d" % (rawccb.pi, rawccb_cur.pi))

        # Tx: verify # packets chained
        if (rawccb_cur.stat_pkts_chain != rawccb.stat_pkts_chain+num_pkts):
            print("stat_pkts_chain check failed old %d new %d expected %d" %
                  (rawccb.stat_pkts_chain, rawccb_cur.stat_pkts_chain, rawccb.stat_pkts_chain+num_pkts))
            app_redir_shared.rawccb_stats_print(tc, rawccb_cur)
            return False
        print("stat_pkts_chain old %d new %d" % 
              (rawccb.stat_pkts_chain, rawccb_cur.stat_pkts_chain))

    app_redir_shared.rawrcb_stats_print(tc, rawrcb_cur)
    app_redir_shared.rawccb_stats_print(tc, rawccb_cur)
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

