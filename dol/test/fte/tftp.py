#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    return

def TestCaseTeardown(tc):
    root = getattr(tc.config, 'flow', None)
    if root is None:
       root = getattr(tc.config.session.iconfig, 'flow', None)

    session = root.GetSession()
    session.SetLabel("FTE_DONE")

    tc.pvtdata.fte_session_aware = False
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
