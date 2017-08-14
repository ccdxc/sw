#! /usr/bin/python3

import pdb
import sys

import infra.engine.modmgr as modmgr
import infra.factory.factory as factory
import infra.config.config as config
import infra.common.objects as objects
import infra.common.defs as defs
from infra.common.glopts import GlobalOptions

from infra.common.logging import logger
from infra.engine.trigger import TrigExpEngine

global modDB

TestCaseDB = objects.ObjectDatabase(None)

def init():
    global modDB
    modDB = modmgr.ModuleDatabase(module_list_file=GlobalOptions.modlist)
    logger.info("Initializing Trigger Infra.")
    TrigExpEngine.start()
    return

def CreateInfraData():
    # TODO: Parse this object from Template:
    # templates/testobjects/framework_data.template
    obj = objects.FrameworkObject()
    obj.Logger          = None
    obj.Factory         = factory
    obj.ConfigStore     = None
    obj.Flows           = None
    obj.TestCases       = []
    obj.TrigExpEngine   = TrigExpEngine
    obj.UserData        = None
    obj.LockAttributes()
    return obj

def ExecuteAllModules():
    module = modDB.getfirst()
    while module != None:
        infra_data = CreateInfraData()
        status = module.main(infra_data)
        if len(infra_data.TestCases):
            TestCaseDB.SetAll(infra_data.TestCases)
        module = modDB.getnext()
 
def PrintSummaryAndExit(report):
    print("\nResult Summary:")
    print("=" * 78)
    print("%-16s %-32s %-6s %-6s %-6s %-6s" %\
          ('Module', 'Name', '', 'Passed', 'Failed', ' Total'))
    print("=" * 78)
    
    module = modDB.getfirst()
    final_result = 0
    while module != None:
        module.PrintResultSummary()
        module_result = module.GetFinalResult()
        #logger.info("Module %s result = %d" % (module.name, module_result))
        final_result += module_result
        module = modDB.getnext()
    
    print("-" * 78)
    print("%-16s %-32s %-6s %6d %6d %6d" %\
          ('Total', '', '', report.passed_count, report.failed_count, 
           report.passed_count + report.failed_count))
    print("-" * 78)
    if final_result != 0:
        print("Final Status = FAIL")
        sys.exit(1)
    
    print("Final Status = PASS")
    sys.exit(0)   

    return
        
def ProcessReport():
    report = TrigExpEngine.get_run_report(20)
    report.show()
    for tcid,tcreport in report.details.items():
        tc = TestCaseDB.Get(tcid)
        tc.status = tcreport.status
        tc.module.UpdateResult(tc)
    PrintSummaryAndExit(report)
    return

def main():
    ExecuteAllModules()
    if GlobalOptions.dryrun == True:
        sys.exit(0)
    ProcessReport()
    return
