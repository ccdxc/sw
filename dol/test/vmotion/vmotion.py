#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi

def Setup(infra, module):
    iterelem = module.iterator.Get()

    if 'flow' in iterelem.__dict__:
        module.testspec.selectors.flow.Extend(iterelem.flow)

    if module.args == None:
        return

    if 'maxflows' in module.args.__dict__:
        module.testspec.selectors.maxflows = module.args.maxflows

    return

def Teardown(infra, module):
    return

def Verify(infra, module):
    return True

def TestCaseSetup(tc):
    tc.pvtdata.acl = None

    #step = tc.step_id
    step = tc.module.iterator.Get().step
    if step == 'FLOW_DROP':
        # step2, Add the NACL drop, Send the packet and verify that it is dropped
        acl = tc.infra_data.ConfigStore.objects.Get('ACL_DIF_ACTION_DROP')
        tc.pvtdata.acl = acl
        acl.UpdateDIF(tc.config.dst.endpoint.intf)
        acl.Show()
        acl.Configure()

    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    acl = tc.pvtdata.acl

    if acl:
        acl.Delete()
    return
