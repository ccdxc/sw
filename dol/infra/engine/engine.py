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

TestCaseDB = objects.ObjectDatabase(logger)

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
        TestCaseDB.SetAll(infra_data.TestCases)
        module = modDB.getnext()
  
def PrintResultSummary():
    print("\nResult Summary:")
    print("=" * 78)
    print("%-24s %-32s %-6s %-6s %-6s" %\
          ('Module', 'Name', 'Passed', 'Failed', ' Total'))
    print("=" * 78)
    module = modDB.getfirst()
    passed = 0
    failed = 0
    total = 0
    while module != None:
        print("%-24s %-32s %6d %6d %6d" %\
              (module.module, module.name,
               module.passed, module.failed, module.total))
        passed += module.passed
        failed += module.failed
        total += module.total
        module = modDB.getnext()
    print("-" * 78)
    print("%-24s %-32s %6d %6d %6d" % ('Total', '', passed, failed, total))
    print("-" * 78)
    return
        
def ProcessReport():
    report = TrigExpEngine.get_run_report(20)
    report.show()
    for tcid,tcreport in report.details.items():
        tc = TestCaseDB.Get(tcid)
        tc.status = tcreport.status
        tc.module.UpdateResult(tc)
    PrintResultSummary()
    if report.failed_count != 0:
        sys.exit(1)
    sys.exit(0)   
    return

def main():
    ExecuteAllModules()
    if GlobalOptions.standalone == True:
        sys.exit(0)
    ProcessReport()
    return
