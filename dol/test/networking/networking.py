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
    modcbs.TestCaseTeardown(tc)
    return
