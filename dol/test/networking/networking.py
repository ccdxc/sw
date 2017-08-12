#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi

def Setup(infra, module):
    module.testspec.config_filter.flow.Extend(module.args.flow)
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    return

def TestCaseVerify(tc):
    return

def TestCaseTeardown(tc):
    return
