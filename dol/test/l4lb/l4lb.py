#! /usr/bin/python3
# Test Module
import pdb
import infra.api.api as InfraApi

import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    return modcbs.Setup(infra, module)

def Teardown(infra, module):
    return modcbs.Teardown(infra, module)

def Verify(infra, module):
    return modcbs.Verify(infra, module)

def TestCaseSetup(tc):
    return modcbs.TestCaseSetup(tc)

def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)

def TestCaseTeardown(tc):
    return modcbs.TestCaseTeardown(tc)
