#! /usr/bin/python3

import pdb

import infra.engine.modmgr as modmgr
import infra.factory.factory as factory
import infra.config.config as config
import infra.common.objects as objects
import infra.common.defs as defs
from infra.common.glopts import GlobalOptions

from infra.common.logging   import logger
from infra.engine.trigger   import TrigExpEngine
from config.store           import Store

global modDB

TestCaseDB = objects.ObjectDatabase(None)

gl_num_pass = 0
gl_num_fail = 0
gl_num_total = 0
gl_junk_recvd = False

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
    obj.ConfigStore     = Store
    obj.TestCases       = []
    obj.TrigExpEngine   = TrigExpEngine
    obj.UserData        = None
    obj.LockAttributes()
    return obj

def ProcessReport():
    report = TrigExpEngine.get_run_report()
    report.show()
    for tcid,tcreport in report.details.items():
        tc = TestCaseDB.Get(tcid)
        tc.status = tcreport.status
        tc.module.UpdateResult(tc)

    global gl_num_pass
    gl_num_pass += report.passed_count
    global gl_num_fail
    gl_num_fail += report.failed_count
    global gl_junk_recvd
    gl_junk_recvd = report.junk_recvd
    TrigExpEngine.reset()
    return

def ExecuteAllModules():
    module = modDB.getfirst()
    while module != None:
        infra_data = CreateInfraData()
        status = module.main(infra_data)
        if len(module.CompletedTestCases):
            TestCaseDB.SetAll(module.CompletedTestCases)
        ProcessReport()
        module = modDB.getnext()
 
def GetSummaryAndResult():
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
          ('Total', '', '',
           gl_num_pass, gl_num_fail, gl_num_pass + gl_num_fail))
    print("-" * 78)

    if final_result != 0:
        print("Final Status = FAIL")
        return 1
    
    global gl_junk_recvd
    if gl_junk_recvd:
        print("Some junk packets where received")
        print("Final Status = FAIL")
        return 1
    
    print("Final Status = PASS")
    return 0

def main():
    ExecuteAllModules()
    return GetSummaryAndResult()
