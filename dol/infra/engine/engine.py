#! /usr/bin/python3

import pdb

import infra.factory.factory        as factory
import infra.config.config          as config
import infra.common.objects         as objects
import infra.common.defs            as defs
import infra.common.timeprofiler    as timeprofiler
import infra.engine.feature         as feature
import infra.engine.modmgr          as modmgr

from infra.common.glopts    import GlobalOptions
from infra.common.logging   import logger
from infra.engine.trigger2  import TriggerEngine
from infra.engine.verif     import VerifEngine
from config.store           import Store
from infra.engine.modmgr    import ModuleStore

SUMMARY_NCOLS = 81

def init():
    feature.Init()
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
    obj.pvtdata         = None
    obj.LockAttributes()
    return obj

def ExecuteAllModules():
    for module in ModuleStore.GetAll():
        infra_data = CreateInfraData()
        module.main(infra_data)
 
def GetSummaryAndResult():
    print("\nResult Summary:")
    print("=" * SUMMARY_NCOLS)
    print("%-16s %-32s %-9s %-6s %-6s %-6s" %\
          ('Feature/Sub', 'Name', 'Result', 'Passed', 'Failed', ' Total'))
    print("=" * SUMMARY_NCOLS)

    npass = 0
    nfail = 0
    ntotal = 0

    final_result = 0
    for module in ModuleStore.GetAll():
        module.PrintResultSummary()
        module_result = module.GetFinalResult()
        final_result += module_result
        npass += module.stats.passed
        nfail += module.stats.failed
        ntotal += module.stats.total
    
    print("-" * SUMMARY_NCOLS)
    totstr = 'Total (Num Modules = %d)' % len(ModuleStore)
    print("%-59s %6d %6d %6d" % (totstr, npass, nfail, ntotal))
    print("-" * SUMMARY_NCOLS)

    #if ntotal == 0:
    #    print("ERROR: No Testcases run !!!")
    #    return 1

    if final_result != 0:
        print("Final Status = FAIL")
        return 1
    
    print("Final Status = PASS")
    return 0

def main():
    timeprofiler.TestTimeProfiler.Start()
    ExecuteAllModules()
    status = GetSummaryAndResult()
    timeprofiler.TestTimeProfiler.Stop()
    return status
