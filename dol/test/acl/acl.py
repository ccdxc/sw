#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi

def Setup(infra, module):

    iterelem = module.iterator.Get()


    if iterelem:
        module.logger.info("ACL id - %s" % iterelem.id)
        if 'flow' in iterelem.__dict__:
            module.testspec.config_filter.flow.Extend(iterelem.flow)

    if module.args == None:
        return

    if 'maxflows' in module.args.__dict__:
        module.testspec.config_filter.maxflows = module.args.maxflows

    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    acl = tc.infra_data.ConfigStore.objects.Get(tc.module.iterator.Get().id)
    tc.pvtdata.acl = acl
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return
