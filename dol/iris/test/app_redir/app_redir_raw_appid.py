# Testcase definition file.

import pdb
import copy
from iris.config.objects.proxycb_service    import ProxyCbServiceHelper
import iris.test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger

cpurx_dpr = 0

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
    tc.SetRetryEnabled(True)
    tc.pvtdata = ObjectDatabase()

    # For this test, we'd like app_redir flow miss pipeline to configure
    # the necessary rawr/rawc CBs so we refrain from doing that here.


    # Clone objects that are needed for verification
    cpurx_dpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"])
    cpurx_dpr.GetMeta()
    cpurx_dpr.GetRingEntries([cpurx_dpr.pi])

    return

def TestCaseVerify(tc):
    global cpurx_dpr

    num_pkts = 1
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = int(tc.module.args.num_pkts)

    num_flow_miss_pkts = 0
    if hasattr(tc.module.args, 'num_flow_miss_pkts'):
        num_flow_miss_pkts = int(tc.module.args.num_flow_miss_pkts)

    # Fetch current values from Platform
    cpurx_dpr_cur = tc.infra_data.ConfigStore.objects.db["CPU_RX_DPR"]
    cpurx_dpr_cur.GetMeta()
    print("CPU_RX_DPR pi old %d new %d" % (cpurx_dpr.pi, cpurx_dpr_cur.pi))

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

