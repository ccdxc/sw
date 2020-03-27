#! /usr/bin/python3
# Networking Module
import pdb

import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.generator as generator
import apollo.test.callbacks.common.modcbs as modcbs
from apollo.config.store import EzAccessStore

def Setup(infra, module):
    if 'WORKFLOW_START' in module.name:
        topo.ChosenFlowObjs.select_objs = True
        topo.ChosenFlowObjs.use_selected_objs = False
        topo.ChosenFlowObjs.SetMaxLimits(module.testspec.selectors.maxlimits)
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase
    # TODO: Ignore tos until all testspecs are updated to take tos from VPC
    tc.AddIgnorePacketField('IP', 'tos')
    ignore_ids = [ 'L2_IPV4_VRIP_ICMP_ECHO_QTAG', 'L3_IPV4_VRIP_ICMP_ECHO_QTAG' ]
    if tc.module.name in ignore_ids:
        tc.AddIgnorePacketField('IP', 'id')
    if tc.config.root.FwdMode == 'IGW_NAPT':
        tc.AddIgnorePacketField('IP', 'src')
        tc.AddIgnorePacketField('UDP', 'sport')
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
        topo.ChosenFlowObjs.select_objs = False
        topo.ChosenFlowObjs.use_selected_objs = True
    elif 'WORKFLOW_END' in tc.module.name:
        topo.ChosenFlowObjs.Reset()
        generator.__read(EzAccessStore.GetDUTNode())
    return True
