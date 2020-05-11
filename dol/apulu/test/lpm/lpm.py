#! /usr/bin/python3
# LPM Module
import pdb
import copy

from infra.common.logging import logger as logger

import apollo.test.callbacks.common.modcbs as modcbs
import apollo.config.utils as utils

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('TCP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase
    if not utils.IsDryRun():
        utils.ReadTestcaseStats(tc.config)
        if hasattr(tc.config, 'statscache') and len(tc.config.statscache):
            tc.pvtdata.statscache = copy.deepcopy(tc.config.statscache)
    return True

def TestCaseTeardown(tc):
    return True

def TestCasePreTrigger(tc):
    return True

def TestCaseStepSetup(tc, step):
    return True

def TestCaseStepTrigger(tc, step):
    return True

def _increment_stats(tc, pre):
    for step in tc.session.steps:
        for desc in step.trigger.descriptors:
            sz = desc.descriptor.object.fields['len']
            pre.increment(sz)
    return

def TestCaseStepVerify(tc, step):
    if not utils.IsDryRun():
        utils.ReadTestcaseStats(tc.config)
        if hasattr(tc.config, 'statscache') and len(tc.config.statscache):
            ret = False
            for objname in tc.config.statscache:
                if not tc.config.statscache[objname] == tc.pvtdata.statscache[objname]:
                    ret = True
                    break
            if ret == False:
                return False
            pre = tc.pvtdata.statscache[objname]
            post = tc.config.statscache[objname]
            _increment_stats(tc, pre)
            if not post == pre:
                # call increment again to handle reinjected packet
                _increment_stats(tc, pre)
                if not post == pre:
                    return False
    return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    return True
