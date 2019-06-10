#! /usr/bin/python3
# Networking Module
import pdb
import apollo.test.callbacks.common.modcbs as modcbs
import apollo.config.objects.dropstats as dropstats

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase

    iterelem = tc.module.iterator.Get()
    if iterelem:
        tc.pvtdata.verify_drop_stats = getattr(iterelem, "drop_stats", False)
        tc.pvtdata.drop_reasons = getattr(iterelem, "drop_reasons", [])
    return True

def TestCaseTeardown(tc):
    return True

def TestCasePreTrigger(tc):
    if tc.pvtdata.verify_drop_stats:
        tc.pvtdata.dropstats = dropstats.DropStatsVerifHelper()
        tc.pvtdata.dropstats.Init(tc)
    return True

def TestCaseStepSetup(tc, step):
    return True

def TestCaseStepTrigger(tc, step):
    return True

def TestCaseStepVerify(tc, step):
    return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    if tc.pvtdata.verify_drop_stats and tc.pvtdata.dropstats.Verify(tc) == False:
        return False
    return True
