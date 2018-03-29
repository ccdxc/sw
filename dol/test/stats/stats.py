#! /usr/bin/python3
import pdb
import test.callbacks.networking.modcbs as modcbs
import config.objects.stats_utils as stats_utils

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    modcbs.Teardown(infra, module)
    return

def Verify(infra, module):
    return modcbs.Verify(infra, module)

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()
    modcbs.TestCaseSetup(tc)
    tc.pvtdata.verify_lif_stats = getattr(iterelem, "lif_stats", False)
    tc.pvtdata.verify_session_stats = getattr(iterelem, "session_stats", False)
    return

def TestCasePreTrigger(tc):
    tc.pvtdata.svh = stats_utils.StatsVerifHelper()
    tc.pvtdata.svh.Init(tc)
    if tc.pvtdata.verify_lif_stats:
        tc.pvtdata.svh.InitLifStats(tc)
    if tc.pvtdata.verify_session_stats:
        tc.pvtdata.svh.InitSessionStats(tc)
    return

def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)
    return

def TestCaseVerify(tc):
    ret = True
    if modcbs.TestCaseVerify(tc) is False:
        ret = False
    if tc.pvtdata.verify_lif_stats and\
       tc.pvtdata.svh.VerifyLifStats(tc) is False:
        ret = False
    if tc.pvtdata.verify_session_stats and\
       tc.pvtdata.svh.VerifySessionStats(tc) is False:
        ret = False
    return ret
