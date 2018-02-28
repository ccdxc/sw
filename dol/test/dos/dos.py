#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    tc.SetRetryEnabled(True)
    modcbs.TestCaseSetup(tc)
    return

def TestCaseTeardown(tc):
    iterelem = tc.module.iterator.Get()
    modcbs.TestCaseTeardown(tc)
    if getattr(iterelem, 'dnu', False) is True:
        if tc.config.src.endpoint.remote == False:
            tc.config.src.endpoint.SetLabel('DO_NOT_USE')
        if tc.config.dst.endpoint.remote == False:
            tc.config.dst.endpoint.SetLabel('DO_NOT_USE')
    return

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def TestCaseStepVerify(tc, step):
    return modcbs.TestCaseStepVerify(tc, step)

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
