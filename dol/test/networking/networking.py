#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi

def Setup(infra, module):
    if module.args == None:
        return

    module.testspec.config_filter.flow.Extend(module.args.flow)
    if 'srcif' in module.args.__dict__:
        module.testspec.config_filter.src.interface.Extend(module.args.srcif)
    else:
        module.testspec.config_filter.src.interface.Extend('filter://type=UPLINK')
    
    if 'dstif' in module.args.__dict__:
        module.testspec.config_filter.dst.interface.Extend(module.args.dstif)
    else:
        module.testspec.config_filter.dst.interface.Extend('filter://type=UPLINK')
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    return

def TestCaseVerify(tc):
    return

def TestCaseTeardown(tc):
    return
