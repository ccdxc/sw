#! /usr/bin/python3
import os
import pdb
import importlib
import subprocess
import operator

import infra.common.dyml        as dyml
import infra.common.defs        as defs
import infra.common.utils       as utils
import infra.common.logging     as logging
import infra.common.objects     as objects
import infra.common.loader      as loader
import infra.factory.testspec   as testspec
import infra.factory.testcase   as testcase

from config.objects.session import SessionHelper
from config.objects.rdma.session import RdmaSessionHelper
from infra.common.logging   import logger
from infra.common.glopts    import GlobalOptions
from config.store import Store  as ConfigStore
from infra.misc import coverage

ModuleIdAllocator = objects.TemplateFieldObject("range/1/8192")
TestCaseIdAllocator = objects.TemplateFieldObject("range/1/65535")

MODULE_CB_INIT_TRACKER  = 'InitTracker'
MODULE_CB_SETUP         = 'Setup'
MODULE_CB_TEARDOWN      = 'Teardown'

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

class Module(objects.FrameworkObject):
    def __init__(self, spec):
        super().__init__()
        self.spec       = spec
        self.name       = spec.name
        self.feature    = getattr(spec, 'feature', '')
        self.GID("%s_%s" % (self.feature, self.name))
        self.package    = spec.package
        self.module     = spec.module
        self.spec       = spec.spec
        self.path       = self.package.replace(".", "/")
        self.ignore     = spec.ignore
        self.iterator   = ModuleIterator(spec.iterate)
        self.args       = getattr(spec, 'args', None)
        self.selectors  = getattr(spec, 'selectors', None)
        self.tracker    = getattr(spec, 'tracker', False)
        self.runorder   = getattr(spec, 'runorder', 65535)
        self.id         = self.runorder << 16 + ModuleIdAllocator.get()
        self.module_hdl = None
        self.infra_data = None
        self.CompletedTestCases = []
        self.abort = False

        self.stats = ModuleStats()
        self.Show()
        return

    def Show(self):
        logger.info("- Module Test  : %s" % self.name)
        logger.info("  - Package    : %s" % self.package)
        logger.info("  - Module     : %s" % self.module)
        logger.info("  - TestSpec   : %s" % self.spec)
        logger.info("  - Ignore     : %s" % self.ignore)
        return

    def __select_config(self):
        utils.LogFunctionBegin(self.logger)
        if hasattr(self.testspec.selectors, "root"):
            obj = self.testspec.selectors.root.Get(ConfigStore)
            module_hdl = loader.ImportModule(obj.meta.package, obj.meta.module)
            assert(module_hdl)
            objs = module_hdl.GetMatchingObjects(self.testspec.selectors)
        else:
            if self.testspec.selectors.IsRdmaSessionBased():
                objs = RdmaSessionHelper.GetMatchingConfigObjects(self.testspec.selectors)
            else:
                objs = SessionHelper.GetMatchingConfigObjects(self.testspec.selectors)

        if GlobalOptions.tcscale is not None:
            objs = objs * int(GlobalOptions.tcscale)
        self.testspec.selectors.roots = objs
        self.logger.info("- Selected %d Matching Objects" % len(objs))
        utils.LogFunctionEnd(self.logger)
        return defs.status.SUCCESS

    def __load_spec(self):
        utils.LogFunctionBegin(self.logger)
        coverage.Init(self)
        self.logger.info("- Loading TEST SPEC = %s" % self.spec)
        self.testspec = testspec.TestSpecObject(self.path, self.spec, self.logger)
        if self.selectors is not None:
            self.testspec.MergeSelectors(self.selectors)
        utils.LogFunctionEnd(self.logger)
        return defs.status.SUCCESS

    def __load(self):
        utils.LogFunctionBegin(self.logger)
        if self.module:
            self.module_hdl = loader.ImportModule(self.package, self.module)
            assert(self.module_hdl)
        utils.LogFunctionEnd(self.logger)

    def GetTracker(self):
        return self.__trackerobj

    def __init_tracker(self):
        self.__trackerobj = None
        if self.tracker is False:
            return
        utils.LogFunctionBegin(self.logger)
        self.__trackerobj = self.RunModuleCallback(MODULE_CB_INIT_TRACKER, self.infra_data, self)
        utils.LogFunctionEnd(self.logger)
        return

    def __unload(self):
        #TODO: Don't unload the modules as verification will
        # have callbacks to the module.
        #del self.module_hdl
        #self.module_hdl = None
        return

    def __update_stats(self, tc):
        if tc.status == defs.status.ERROR:
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

    def IsIgnore(self):
        return self.ignore

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

        feature = self.feature[:16]
        name = self.name[:32]
        print("%-16s %-32s %-6s %6d %6d %6d" %\
              (feature, name, status,
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
        return defs.status.SUCCESS

    def __setup_callback(self):
        utils.LogFunctionBegin(self.logger)
        self.RunModuleCallback(MODULE_CB_SETUP, self.infra_data, self)
        utils.LogFunctionEnd(self.logger)
        return

    def __update_results(self, testcase):
        return

    def __debug_testcase(self, testcase):
        return

    def __execute_testcase(self, testcase):
        # Process trigger section of testspec
        self.infra_data.TriggerEngine.Trigger(testcase)
        self.__debug_testcase(testcase)
        self.__update_results(testcase)
        return

    def __is_tcid_filter_match(self, tcid):
        if GlobalOptions.tcid is None:
            return True

        if not isinstance(GlobalOptions.tcid, int):
            GlobalOptions.tcid = utils.ParseInteger(GlobalOptions.tcid)
    
        return tcid == GlobalOptions.tcid

    def __execute(self):
        for root in self.testspec.selectors.roots:
            tcid = TestCaseIdAllocator.get()
            if not self.__is_tcid_filter_match(tcid): continue
            tc = testcase.TestCase(tcid, root, self)
            self.__execute_testcase(tc)
            self.CompletedTestCases.append(tc)
            if self.tracker and tc.status == defs.status.ERROR:
                self.abort = True
                break
        return

    def __teardown_callback(self):
        utils.LogFunctionBegin(self.logger)
        self.RunModuleCallback(MODULE_CB_TEARDOWN, self.infra_data, self)
        utils.LogFunctionEnd(self.logger)
        return

    def __process_results(self):
        for tc in self.CompletedTestCases:
            self.__update_stats(tc)
        return

    def main(self, infra_data):
        self.infra_data = infra_data
        prefix = "%s/%s" % (self.feature, self.name)
        self.logger = logging.Logger(level=logging.levels.INFO, stdout=True,
                                     name=prefix)
        self.infra_data.Logger = self.logger
        self.__load()
        self.__init_tracker()
        while self.iterator.End() is False and self.abort is False:
            self.logger.info("========== Starting Test Module =============")
            self.logger.info("Starting new Iteration")
            self.__load_spec()
            self.__setup_callback()
            self.__select_config()
            self.__execute()
            self.__teardown_callback()
            self.iterator.Next()

        self.__unload()
        self.stats.total = len(self.CompletedTestCases)
        self.__process_results()
        return

class ModuleDatabase:
    def __init__(self):
        self.db = {}
        if GlobalOptions.test != None:
            GlobalOptions.test = GlobalOptions.test.split(',')
        if GlobalOptions.pkglist != None:
            GlobalOptions.pkglist = GlobalOptions.pkglist.split(',')
        return

    def __len__(self):
        return len(self.db)

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

    def __is_pkg_match(self, pkg):
        if GlobalOptions.pkglist == None:
            return True

        if pkg in GlobalOptions.pkglist:
            return True

        return False

    def __add(self, pmod):
        if GlobalOptions.test and pmod.name in GlobalOptions.test:
            pmod.enable = True

        if GlobalOptions.pkglist and pmod.package in GlobalOptions.pkglist:
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

        if not self.__is_pkg_match(pmod.package):
            return

        module = Module(pmod)
        if module.GID() in self.db:
            logger.error("Duplicate Test : %s" % module.GID())
            assert(0)
        self.db[module.GID()] = module
        return

    def GetAll(self):
        return sorted(self.db.values(), key=operator.attrgetter('runorder'))

    def Add(self, spec):
        self.__add(spec)
        return

ModuleStore = ModuleDatabase()
