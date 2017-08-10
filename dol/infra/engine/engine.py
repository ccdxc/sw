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


def AllocUserData():
    obj = objects.FrameworkObject()
    obj.spec = None
    obj.logger = None
    obj.LockAttributes()
    return obj


def AllocFrameworkData():
    # TODO: Parse this object from Template:
    # templates/testobjects/framework_data.template
    obj = objects.FrameworkObject()
    obj.TestModuleID = None
    obj.Logger = None
    obj.Factory = factory
    obj.TestSpec = None
    obj.CfgObjectStore = None
    obj.MemObjectStore = None
    obj.CfgFlows = None
    obj.TestCases = []
    obj.TrigExpEngine = TrigExpEngine
    obj.UserData = None
    obj.LockAttributes()
    return obj


def main():
    module = modDB.getfirst()
    while module != None:
        fwdata = AllocFrameworkData()
        fwdata.UserData = AllocUserData()
        status = module.main(fwdata)
        module = modDB.getnext()

    if GlobalOptions.standalone == True:
        sys.exit(0)
    
    report = TrigExpEngine.get_run_report(20)
    report.show()
    if report.failed_count != 0:
        sys.exit(1)
    sys.exit(0)
