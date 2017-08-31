#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi

def Setup(infra, module):
    iterelem = module.iterator.Get()

    if iterelem:
        if 'flow' in iterelem.__dict__:
            module.testspec.config_filter.flow.Extend(iterelem.flow)

        if 'srcif' in iterelem.__dict__:
            module.testspec.config_filter.src.interface.Extend(iterelem.srcif)
        else:
            module.testspec.config_filter.src.interface.Extend('filter://any')
        
        if 'dstif' in iterelem.__dict__:
            module.testspec.config_filter.dst.interface.Extend(iterelem.dstif)
        else:
            module.testspec.config_filter.dst.interface.Extend('filter://any')

        if 'segment' in iterelem.__dict__:
            module.testspec.config_filter.src.segment.Extend(iterelem.segment)
            module.testspec.config_filter.dst.segment.Extend(iterelem.segment)

        if 'srcseg' in iterelem.__dict__:
            module.testspec.config_filter.src.segment.Extend(iterelem.srcseg)

        if 'dstseg' in iterelem.__dict__:
            module.testspec.config_filter.dst.segment.Extend(iterelem.dstseg)

    if module.args == None:
        return

    if 'maxflows' in module.args.__dict__:
        module.testspec.config_filter.maxflows = module.args.maxflows

    return

def Teardown(infra, module):
    return

def Verify(infra, module):
    if 'RECIRC' in module.name and module.stats.total == 0:
        return False
    return True

def TestCaseSetup(tc):
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return
