# Testcase definition file.

import pdb
import copy
from config.objects.proxycb_service    import ProxyCbServiceHelper
import test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger


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

    # For this test, we'd like app_redir flow miss pipeline to configure
    # the necessary rawr/rawc CBs so we refrain from doing that here.


    # Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    rnmpr_small = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"])
    rnmpr_small.Configure()
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["ARQ"])
    arq.Configure()
    
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(rnmpr_small)
    tc.pvtdata.Add(arq)
    return

def TestCaseVerify(tc):

    num_pkts = 1
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = int(tc.module.args.num_pkts)

    # Fetch current values from Platform
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    rnmpr_small = tc.pvtdata.db["RNMPR_SMALL"]
    arq = tc.pvtdata.db["ARQ"]

    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    rnmpr_small_cur = tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"]
    rnmpr_small_cur.Configure()
    arq_cur = tc.infra_data.ConfigStore.objects.db["ARQ"]
    arq_cur.Configure()

    # Verify PI for RNMDR got incremented
    if (rnmdr_cur.pi != rnmdr.pi+num_pkts):
        print("RNMDR pi check failed old %d new %d expected %d" % 
                     (rnmdr.pi, rnmdr_cur.pi, rnmdr.pi+num_pkts))
        return False
    print("RNMDR pi old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))

    # Verify PI for RNMPR or RNMPR_SMALL got incremented
    if ((rnmpr_cur.pi+rnmpr_small_cur.pi) != (rnmpr.pi+rnmpr_small.pi+num_pkts)):
        print("RNMPR pi check failed old %d new %d expected %d" %
                  (rnmpr.pi+rnmpr_small.pi, rnmpr_cur.pi+rnmpr_small_cur.pi,
                   rnmpr.pi+rnmpr_small.pi+num_pkts))
        return False
    print("RNMPR pi old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
    print("RNMPR_SMALL old %d new %d" % (rnmpr_small.pi, rnmpr_small_cur.pi))

    # Rx: verify PI for ARQ got incremented
    if (arq_cur.pi != arq.pi+num_pkts):
        print("ARQ pi check failed old %d new %d expected %d" %
                   (arq.pi, arq_cur.pi, arq.pi+num_pkts))
        return False
    print("ARQ pi old %d new %d" % (arq.pi, arq_cur.pi))

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

