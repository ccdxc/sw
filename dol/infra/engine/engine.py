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


def main():
    module = modDB.getfirst()
    while module != None:
        infra_data = CreateInfraData()
        status = module.main(infra_data)
        module = modDB.getnext()

    if GlobalOptions.standalone == True:
        sys.exit(0)
    
    report = TrigExpEngine.get_run_report(20)
    report.show()
    logger.info("SUMMARY: TOTAL=%d PASS=%d FAIL=%d" %\
                (report.passed_count + report.failed_count,
                 report.passed_count, report.failed_count))
    if report.failed_count != 0:
        sys.exit(1)
    sys.exit(0)
