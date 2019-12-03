#! /usr/bin/python3
# Networking Module
import pdb

import apollo.config.utils as utils
import apollo.test.callbacks.common.modcbs as modcbs

def Setup(infra, module):
    if 'WORKFLOW_START' in module.name:
        utils.CachedObjs.select_objs = True
        utils.CachedObjs.setMaxLimits(module.testspec.selectors.maxlimits)
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase
    return True

def TestCaseTeardown(tc):
    return True

def TestCasePreTrigger(tc):
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
    if 'WORKFLOW_START' in tc.module.name:
        utils.CachedObjs.select_objs = False
        utils.CachedObjs.use_selected_objs = True
    elif 'WORKFLOW_END' in tc.module.name:
        utils.CachedObjs.reset()
    return True
