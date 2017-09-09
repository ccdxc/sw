#! /usr/bin/python3
import os
import pdb
import importlib

import infra.common.dyml        as dyml
import infra.common.defs        as defs
import infra.common.utils       as utils
import infra.common.logging     as logging
import infra.common.objects     as objects
import infra.common.loader      as loader
import infra.factory.testspec   as testspec

from config.objects.session import SessionHelper
from config.objects.rdma.session import RdmaSessionHelper
from infra.common.logging   import logger
from infra.common.glopts    import GlobalOptions

ModuleIdAllocator = objects.TemplateFieldObject("range/1/8192")

MODULE_CB_SETUP     = 'Setup'
MODULE_CB_TEARDOWN  = 'Teardown'

class ModuleStats:
    def __init__(self):
        self.passed     = 0
        self.failed     = 0
        self.ignored    = 0
        self.total      = 0

class ModuleIterator:
    def __init__(self, elems):
        self.elems  = elems
        self.idx    = 0
        return

    def Get(self):
        return self.elems[self.idx]

    def Next(self):
        self.idx += 1
        return

    def End(self):
        return self.idx == len(self.elems)

class Module:
    def __init__(self, parsedata):
        self.parsedata = parsedata

        self.name       = parsedata.name
        self.package    = parsedata.package
        self.module     = parsedata.module
        self.spec       = parsedata.spec
        self.path       = self.package.replace(".", "/")
        self.ignore     = parsedata.ignore
        self.iterator   = ModuleIterator(parsedata.iterate)
        self.args       = None 
        if 'args' in parsedata.__dict__:
            self.args = parsedata.args
        self.id         = ModuleIdAllocator.get()
        self.module_hdl = None
        self.infra_data = None
        self.CompletedTestCases = []

        self.stats = ModuleStats()
        return

    def __select_config(self):
        utils.LogFunctionBegin(self.logger)
        #objs = SessionHelper.GetMatchingConfigObjects(self.testspec.selectors)
        objs = RdmaSessionHelper.GetMatchingConfigObjects(self.testspec.selectors)
        self.testspec.selectors.matching_objects = objs
        self.logger.info("- Selected %d Matching Objects" % len(objs))
        utils.LogFunctionEnd(self.logger)
        return defs.status.SUCCESS

    def __load_spec(self):
        utils.LogFunctionBegin(self.logger)
        self.logger.info("- Loading TEST SPEC = %s" % self.spec)
        self.testspec = testspec.TestSpecObject(self.path, self.spec)
        utils.LogFunctionEnd(self.logger)
        return defs.status.SUCCESS

    def __load(self):
        utils.LogFunctionBegin(self.logger)
        if self.module:
            self.module_hdl = loader.ImportModule(self.package, self.module)
            assert(self.module_hdl)
        utils.LogFunctionEnd(self.logger)

    def __unload(self):
        #TODO: Don't unload the modules as verification will
        # have callbacks to the module.
        #del self.module_hdl
        #self.module_hdl = None
        return

    def UpdateResult(self, tc):
        if tc.status == 'Failed':
            if self.ignore:
                self.stats.ignored += 1
            self.stats.failed += 1
        else:
            self.stats.passed += 1
        return

    def GetFinalResult(self):
        if self.stats.total == 0:
            return 1

        if self.stats.failed == 0:
            return 0
        assert(self.stats.failed >= self.stats.ignored)
        net_fail = self.stats.failed - self.stats.ignored
        if net_fail > 0:
            return 1
        return 0

    def PrintResultSummary(self):
        status = ''
        if self.stats.total == 0:
            status = 'Error'
        elif self.ignore:
            status = 'Ignore'
        elif self.GetFinalResult():
            status = 'Fail'
        else:
            status = 'Pass'

        mod = self.module[:16]
        name = self.name[:32]
        print("%-16s %-32s %-6s %6d %6d %6d" %\
              (mod, name, status,
               self.stats.passed, self.stats.failed,
               self.stats.total))
        return

    def GetNumFailed(self):
        if self.ignore:
            return 0
        return self.failed

    def RunModuleCallback(self, cb, *args):
        if self.module_hdl == None:
            return
        if hasattr(self.module_hdl, cb):
            cb_handle = getattr(self.module_hdl, cb)
            assert(cb_handle != None)
            return cb_handle(*args)
        return



    def __setup_callback(self):
        utils.LogFunctionBegin(self.logger)
        self.RunModuleCallback(MODULE_CB_SETUP, self.infra_data, self)
        utils.LogFunctionEnd(self.logger)
        return

    def __generate(self):
        utils.LogFunctionBegin(self.logger)
        status = self.infra_data.Factory.Generate(self.infra_data, self)
        utils.LogFunctionEnd(self.logger, status)
        return status

    def __update_results(self, testcase):
        return

    def __debug_testcase(self, testcase):
        return

    def __execute_testcase(self, testcase):
        testcase.TriggerCallback()
        testcase.status = self.infra_data.TrigExpEngine.run_test_case(testcase)
        self.__debug_testcase(testcase)
        self.__update_results(testcase)
        return

    def __execute(self):
        for testcase in self.infra_data.TestCases:
            self.__execute_testcase(testcase)
        self.CompletedTestCases.extend(self.infra_data.TestCases)
        self.infra_data.TestCases = []
        return

    def __teardown_callback(self):
        utils.LogFunctionBegin(self.logger)
        self.RunModuleCallback(MODULE_CB_TEARDOWN, self.infra_data, self)
        utils.LogFunctionEnd(self.logger)
        return

    def main(self, infra_data):
        self.infra_data = infra_data
        self.logger = logging.Logger(level=logging.levels.INFO, stdout=True,
                                     name="%s" % self.name)
                                     #log_file='logs/' + self.name + ".log")
        self.infra_data.Logger = self.logger
        self.__load()
        while self.iterator.End() == False:
            self.logger.info("========== Starting Test Module =============")
            self.logger.info("Starting new Iteration")
            self.__load_spec()
            self.__setup_callback()
            self.__select_config()
            self.__generate()
            self.__execute()
            self.__teardown_callback()
            self.iterator.Next()

        self.__unload()
        self.stats.total = len(self.CompletedTestCases)
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
        if GlobalOptions.test != None:
            GlobalOptions.test = GlobalOptions.test.split(',')
        self.__add_all()
        return

    def __is_test_match(self, name):
        if GlobalOptions.test == None:
            return True

        if name in GlobalOptions.test:
            return True

        return False

    def __is_module_match(self, module):
        if GlobalOptions.module == None:
            return True

        if module in GlobalOptions.module:
            return True

        return False

    def __add(self, pmod):
        if GlobalOptions.test and pmod.name in GlobalOptions.test:
            pmod.enable = True

        if pmod.enable == False:
            return

        if 'ignore' not in pmod.__dict__:
            pmod.ignore = False

        if 'iterate' not in pmod.__dict__:
            pmod.iterate = [ None ]

        if not self.__is_test_match(pmod.name):
            return

        if not self.__is_module_match(pmod.module):
            return

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
