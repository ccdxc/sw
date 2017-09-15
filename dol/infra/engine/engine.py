#! /usr/bin/python3

import pdb

import infra.engine.modmgr as modmgr
import infra.factory.factory as factory
import infra.config.config as config
import infra.common.objects as objects
import infra.common.defs as defs
from infra.common.glopts import GlobalOptions

from infra.common.logging   import logger
from infra.engine.trigger2  import TriggerEngine
from infra.engine.verif     import VerifEngine
from config.store           import Store

global modDB

def init():
    global modDB
    modDB = modmgr.ModuleDatabase(module_list_file=GlobalOptions.modlist)
    return

def CreateInfraData():
    # TODO: Parse this object from Template:
    # templates/testobjects/framework_data.template
    obj = objects.FrameworkObject()
    obj.Logger          = None
    obj.Factory         = factory
    obj.ConfigStore     = Store
    obj.TriggerEngine   = TriggerEngine
    obj.VerifEngine     = VerifEngine
    obj.UserData        = None
    obj.LockAttributes()
    return obj

def ExecuteAllModules():
    module = modDB.getfirst()
    while module != None:
        infra_data = CreateInfraData()
        module.main(infra_data)
        module = modDB.getnext()
 
def GetSummaryAndResult():
    print("\nResult Summary:")
    print("=" * 78)
    print("%-16s %-32s %-6s %-6s %-6s %-6s" %\
          ('Module', 'Name', '', 'Passed', 'Failed', ' Total'))
    print("=" * 78)

    npass = 0
    nfail = 0
    ntotal = 0
    gl_junk_recvd = False

    module = modDB.getfirst()
    final_result = 0
    while module != None:
        module.PrintResultSummary()
        module_result = module.GetFinalResult()
        final_result += module_result
        npass += module.stats.passed
        nfail += module.stats.failed
        ntotal += module.stats.total
        module = modDB.getnext()
    
    print("-" * 78)
    print("%-16s %-32s %-6s %6d %6d %6d" %\
          ('Total', '', '', npass, nfail, ntotal))
    print("-" * 78)

    if final_result != 0:
        print("Final Status = FAIL")
        return 1
    
    if gl_junk_recvd:
        print("Some junk packets where received")
        print("Final Status = FAIL")
        return 1
    
    print("Final Status = PASS")
    return 0

def main():
    ExecuteAllModules()
    return GetSummaryAndResult()
