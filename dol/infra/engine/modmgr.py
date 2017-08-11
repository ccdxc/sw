#! /usr/bin/python3
import os
import pdb
import importlib

import infra.common.dyml        as dyml
import infra.common.defs        as defs
import infra.common.utils       as utils
import infra.common.logging     as logging
import infra.common.objects     as objects
import infra.factory.testspec   as testspec

from config.objects.session import SessionHelper
from infra.common.logging   import logger
from infra.common.glopts    import GlobalOptions

ModuleIdAllocator = objects.TemplateFieldObject("range/1/8192")

MODULE_CB_SETUP     = 'setup'
MODULE_CB_TEARDOWN  = 'teardown'

class Module:
    def __init__(self, parsedata):
        self.parsedata = parsedata

        self.name       = parsedata.name
        self.package    = parsedata.package
        self.module     = parsedata.module
        self.spec       = parsedata.spec
        self.path       = self.package.replace(".", "/")

        self.TestModuleID = ModuleIdAllocator.get()
        self.modhdl = None
        self.fwdata = None
        self.usrdata = None
        return

    def __select_config(self):
        utils.LogFunctionBegin(self.logger)
        self.fwdata.CfgFlows =\
                    SessionHelper.GetFlows(self.fwdata.TestSpec.config_filter)
        self.logger.info("- Selected %d FLOWS" % len(self.fwdata.CfgFlows))
        utils.LogFunctionEnd(self.logger)
        return defs.status.SUCCESS

    def __load_spec(self):
        utils.LogFunctionBegin(self.logger)
        self.logger.info("- Loading TEST SPEC = %s" % self.spec)
        self.fwdata.TestSpec = testspec.TestSpec(self.path, self.spec)
        utils.LogFunctionEnd(self.logger)
        return defs.status.SUCCESS

    def __load(self):
        utils.LogFunctionBegin(self.logger)
        module_name = "%s.%s" % (self.parsedata.package, self.module)
        self.modhdl = importlib.import_module(module_name)
        assert(self.modhdl)
        utils.LogFunctionEnd(self.logger)

    def __unload(self):
        del self.modhdl
        self.modhdl = None
        return

    def __module_cb(self, cb):
        cb_handle = getattr(self.modhdl, cb)
        assert(cb_handle != None)
        return cb_handle(self.fwdata, self.usrdata)

    def __setup(self):
        utils.LogFunctionBegin(self.logger)
        status = self.__module_cb(MODULE_CB_SETUP)
        utils.LogFunctionEnd(self.logger, status)
        return status

    def __generate(self):
        utils.LogFunctionBegin(self.logger)
        status = self.fwdata.Factory.Generate(self.fwdata)
        utils.LogFunctionEnd(self.logger, status)
        return status

    def __update_results(self, testcase):
        return

    def __debug_testcase(self, testcase):
        return

    def __execute_testcase(self, testcase):
        testcase.status = self.fwdata.TrigExpEngine.run_test_case(testcase)
        self.__debug_testcase(testcase)
        self.__update_results(testcase)
        return

    def __execute(self):
        for testcase in self.fwdata.TestCases:
            self.__execute_testcase(testcase)
        return

    def __teardown(self):
        utils.LogFunctionBegin(self.logger)
        status = self.__module_cb(MODULE_CB_TEARDOWN)
        utils.LogFunctionEnd(self.logger, status)
        return defs.status.SUCCESS

    def main(self, fwdata):
        fwdata.TestModuleID = self.TestModuleID
        self.fwdata = fwdata
        self.usrdata = fwdata.UserData
        self.logger = logging.Logger(level=logging.levels.INFO,
                                     name="MOD:%s" % self.name)
        self.fwdata.Logger = self.logger

        self.logger.info("========== Starting Test Module =============")
        self.__load()
        self.__load_spec()
        self.__setup()
        self.__select_config()
        self.__generate()
        self.__execute()
        self.__teardown()
        self.__unload()
        return


class ModuleListParser:
    def __init__(self, module_list_file):
        self.module_list_file = module_list_file
        self.parsed_module_list = []
        self.parse()
        return

    def parse(self):
        full_path = utils.GetFullPath(self.module_list_file)
        genobj = dyml.main(full_path)
        for elem in genobj.modules:
            logger.debug("Parsed MODULE:%s" % elem.module.name)
            self.parsed_module_list.append(elem.module)
            elem.show()
        return


class ModuleDatabase:
    def __init__(self, module_list_file=defs.DEFAULT_MODULES_LIST_FILE):
        self.db = []
        self.parser = ModuleListParser(module_list_file)
        self.__add_all()
        return

    def __add(self, pmod):
        if GlobalOptions.test == pmod.name:
            pmod.enable = True

        if pmod.enable == False:
            return

        if GlobalOptions.test == None or\
           GlobalOptions.test == pmod.name:
            module = Module(pmod)
            self.db.append(module)
        return

    def __add_all(self):
        for pmod in self.parser.parsed_module_list:
            self.__add(pmod)
        return

    def getnext(self):
        if self.iter == len(self.db):
            return None
        mod = self.db[self.iter]
        self.iter += 1
        return mod

    def getfirst(self):
        self.iter = 0
        return self.getnext()
