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
    modcbs.TestCaseSetup(tc)
    return

def TestCasePreTrigger(tc):
    modcbs.TestCasePreTrigger(tc)
    return

def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)
    return

def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)
