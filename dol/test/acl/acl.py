#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi

def Setup(infra, module):

    iterelem = module.iterator.Get()

    module.logger.info("ACL id - %s" % iterelem.id)
    if 'flow' in iterelem.__dict__:
        module.testspec.selectors.flow.Extend(iterelem.flow)

    if 'srcseg' in iterelem.__dict__:
        module.testspec.selectors.src.segment.Extend(iterelem.srcseg)

    if module.args == None:
        return

    if 'maxflows' in module.args.__dict__:
        module.testspec.selectors.maxflows = module.args.maxflows

    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    acl = tc.infra_data.ConfigStore.objects.Get(tc.module.iterator.Get().id)
    tc.pvtdata.acl = acl

    # Update the ACL parameters based on the testcase
    acl.UpdateFromTCConfig(tc.config.flow, tc.config.src.endpoint, tc.config.dst.endpoint,\
                           tc.config.src.segment, tc.config.src.tenant)

    acl.Configure()
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    acl = tc.pvtdata.acl

    acl.Delete()
    return
