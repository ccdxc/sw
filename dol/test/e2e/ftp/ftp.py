#! /usr/bin/python3
# Test Module

import os 
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    from pyftpdlib import handlers
    handlers.PassiveDTP.timeout = None
    handlers.ActiveDTP.timeout = None
    pass

def TestCaseTeardown(tc):
    os.system("rm -f ftp_file.txt")
    pass

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def TestCaseStepVerify(tc, step):
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
