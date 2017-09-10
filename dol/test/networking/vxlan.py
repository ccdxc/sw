#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    return

def Verify(infra, module):
    if 'RECIRC' in module.name and module.stats.total == 0:
        return False
    return True

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()
    tc.pvtdata.priotag = False
    if 'priotag' in iterelem.__dict__:
        tc.pvtdata.priotag = iterelem.priotag
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return
