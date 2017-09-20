#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()
    tc.pvtdata.priotag = False
    if 'priotag' in iterelem.__dict__:
        tc.pvtdata.priotag = iterelem.priotag
    return

def TestCaseTeardown(tc):
    if tc.config.flow.IsFteEnabled():
        tc.config.flow.SetLabel("FTE_DONE")
    return
