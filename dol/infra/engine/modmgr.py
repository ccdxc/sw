#! /usr/bin/python3
import os
import pdb
import copy
import importlib
import subprocess
import operator
import random

import infra.common.dyml        as dyml
import infra.common.defs        as defs
import infra.common.utils       as utils
import infra.common.logging     as logging
import infra.common.objects     as objects
import infra.common.loader      as loader
import infra.factory.testspec   as testspec
import infra.factory.testcase   as testcase

from infra.factory.store import FactoryStore as FactoryStore
from config.objects.session import SessionHelper
from config.objects.rdma.session import RdmaSessionHelper
from config.objects.e2e import E2EHelper
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
        self.Reset()
        return

    def Reset(self):
        self.passed     = 0
        self.failed     = 0
        self.ignored    = 0
        self.total      = 0
        return

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

    def Reset(self):
        self.idx = 0

class ModulePrivateData:
    def __init__(self):
        return

class Module(objects.FrameworkObject):
    def __init__(self, spec):
        super().__init__()
        self.spec       = spec
        self.name       = spec.name
        self.feature    = getattr(spec, 'feature', '')
        self.GID("%s_%s" % (self.feature, self.name))
        self.enable     = spec.enable
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
        self.rtl        = getattr(spec, 'rtl', True)
        self.perf    = getattr(spec, 'perf', False)
        self.pendol     = getattr(spec, 'pendol', False)
        self.tcscale    = getattr(spec, 'tcscale', None)
        self.modscale    = getattr(spec, 'modscale', None)
        self.id         = self.runorder << 16 + ModuleIdAllocator.get()
        self.module_hdl = None
        self.infra_data = None
        self.CompletedTestCases = []
        self.abort = False

        self.stats = ModuleStats()
        self.Show()
        return

    def InitCopy(self, inst):
        self.name = "%s_%03d" % (self.name, inst)
        self.GID("%s_%s" % (self.feature, self.name))
        return

    def IsPendolHeaderEnabled(self):
        return self.pendol

    def Show(self):
        logger.info("- Module Test  : %s" % self.name)
        logger.info("  - Package    : %s" % self.package)
        logger.info("  - Module     : %s" % self.module)
        logger.info("  - TestSpec   : %s" % self.spec)
        logger.info("  - Ignore     : %s" % self.ignore)
        return


    def __load_spec(self):
        logger.LogFunctionBegin()
        coverage.Init(self)
        logger.info("- Loading TEST SPEC = %s" % self.spec)
        self.testspec = testspec.TestSpecObject(self.path, self.spec, logger)
        self.testspec.MergeSelectors(self.selectors)
        self.testspec.DeriveLimits()
        logger.LogFunctionEnd()
        return defs.status.SUCCESS

    def __load(self):
        logger.LogFunctionBegin()
        if self.module:
            self.module_hdl = loader.ImportModule(self.package, self.module)
            assert(self.module_hdl)
        logger.LogFunctionEnd()

    def GetTracker(self):
        return self.__trackerobj

    def __init_tracker(self):
        self.__trackerobj = None
        if self.tracker is False:
            return
        logger.LogFunctionBegin()
        self.__trackerobj = self.RunModuleCallback(MODULE_CB_INIT_TRACKER, self.infra_data, self)
        logger.LogFunctionEnd()
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
        if self.enable == False:
            return 0
        if self.stats.total == 0:
            if GlobalOptions.tcid != None:
                # When running in 1 testcase mode, tests can be skipped as 
                # they did not match the testcase id. Dont treat this as error.
                return 0
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
        if self.enable == False:
            status = 'Disabled'
        elif self.stats.total == 0:
            if GlobalOptions.tcid != None:
                status = 'Skipped'
            else:
                status = 'Error'
        elif self.ignore:
            status = 'Ignore'
        elif self.GetFinalResult():
            status = 'Fail'
        else:
            status = 'Pass'

        feature = self.feature[:16]
        name = self.name[:32]
        print("%-16s %-32s %-9s %6d %6d %6d" %\
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
        logger.LogFunctionBegin()
        self.RunModuleCallback(MODULE_CB_SETUP, self.infra_data, self)
        logger.LogFunctionEnd()
        return

    def __update_results(self, testcase):
        return

    def __debug_testcase(self, testcase):
        return

    def _execute_testcase(self, testcase):
        # Process trigger section of testspec
        testcase.trigger_engine.Trigger(testcase)
        self.__debug_testcase(testcase)
        self.__update_results(testcase)
        return

    def _is_tcid_filter_match(self, tcid):
        if GlobalOptions.tcid is None:
            return True

        if isinstance(GlobalOptions.tcid, list):
            if tcid in GlobalOptions.tcid:
                if tcid == GlobalOptions.tcid[-1]:
                    GlobalOptions.alltc_done = True
                return True
            return False
            
        if not isinstance(GlobalOptions.tcid, int):
            GlobalOptions.tcid = utils.ParseInteger(GlobalOptions.tcid)
    
        if tcid == GlobalOptions.tcid:
            GlobalOptions.alltc_done = True
            return True
        return False

    def __execute(self):
        matching_tcsets = []
        for root in self.testspec.selectors.roots:
            tcid = TestCaseIdAllocator.get()
            if not self._is_tcid_filter_match(tcid): continue
            matching_tcsets.append((tcid,root))

        nloops = 1
        if GlobalOptions.tcscale:
            nloops = int(GlobalOptions.tcscale)
        if self.tcscale:
            nloops = self.tcscale

        for tcid,root in matching_tcsets:
            for loopid in range(nloops):
                looptc = testcase.GetTestCase(tcid, root, self, loopid)
                self._execute_testcase(looptc)
                self.CompletedTestCases.append(looptc)
                if self.tracker and looptc.status == defs.status.ERROR:
                    self.abort = True
                    break
        return

    def __teardown_callback(self):
        logger.LogFunctionBegin()
        self.RunModuleCallback(MODULE_CB_TEARDOWN, self.infra_data, self)
        logger.LogFunctionEnd()
        return

    def __process_results(self):
        self.stats.Reset()
        self.stats.total = len(self.CompletedTestCases)
        for tc in self.CompletedTestCases:
            self.__update_stats(tc)
        return

    def main(self, infra_data):
        if self.enable == False:
            return

        self.infra_data = infra_data
        prefix = "%s/%s" % (self.feature, self.name)
        logger.SetModule(prefix)
        self.__load()
        self.__init_tracker()
        while self.iterator.End() is False and\
              self.abort is False and\
              GlobalOptions.alltc_done == False:
            self.pvtdata = ModulePrivateData()
            logger.info("========== Starting Test Module =============")
            logger.info("Starting new Iteration")
            self.__load_spec()
            self.__setup_callback()
            #This should be cleaned up.
            test_spec = self.testspec.type.Get(FactoryStore)
            if test_spec.meta.id == "E2E_TESTCASE":
                runner = E2EModuleRunner(self)
            else:
                runner = DolModuleRunner(self)
            runner.Run()
            self.__teardown_callback()
            self.iterator.Next()

        self.__unload()
        self.__process_results()
        self.iterator.Reset()
        return

class ModuleRunner:
    def __init__(self, module):
        self.module = module
    
    def Run(self):
        pass


class DolModuleRunner(ModuleRunner):
    def __init__(self, module):
        super().__init__(module)
        
    def __select_config(self):
        logger.LogFunctionBegin()
        if hasattr(self.module.testspec.selectors, "root"):
            obj = self.module.testspec.selectors.root.Get(ConfigStore)
            module_hdl = loader.ImportModule(obj.meta.package, obj.meta.module)
            assert(module_hdl)
            objs = module_hdl.GetMatchingObjects(self.module.testspec.selectors)
        else:
            if self.module.testspec.selectors.IsRdmaSessionBased():
                objs = RdmaSessionHelper.GetMatchingConfigObjects(self.module.testspec.selectors)
            else:
                objs = SessionHelper.GetMatchingConfigObjects(self.module.testspec.selectors)

        self.module.testspec.selectors.roots = objs
        if len(objs) == 0:
            logger.error("- Selected %d Matching Objects" % len(objs))
        else:
            logger.info("- Selected %d Matching Objects" % len(objs))
        logger.LogFunctionEnd()
        return defs.status.SUCCESS    

    def __execute(self):
        matching_tcsets = []
        for root in self.module.testspec.selectors.roots:
            tcid = TestCaseIdAllocator.get()
            if not self.module._is_tcid_filter_match(tcid): continue
            matching_tcsets.append((tcid,root))

        nloops = 1
        if GlobalOptions.tcscale:
            nloops = int(GlobalOptions.tcscale)
        if self.module.tcscale:
            nloops = self.module.tcscale

        for tcid,root in matching_tcsets:
            for loopid in range(nloops):
                looptc = testcase.GetTestCase(tcid, root, self.module, loopid)
                self.module._execute_testcase(looptc)
                self.module.CompletedTestCases.append(looptc)
                if self.module.tracker and looptc.status == defs.status.ERROR:
                    self.module.abort = True
                    break
        return
    
    def Run(self):
        self.__select_config()
        self.__execute()

class E2EModuleRunner(ModuleRunner):
    def __init__(self, module):
        super().__init__(module)
    
    def Run(self):
        tcid = TestCaseIdAllocator.get()
        tc = testcase.GetTestCase(tcid, None, self.module, 0)
        tc.SetUp()
        self.module._execute_testcase(tc)
        self.module.CompletedTestCases.append(tc)
 
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

        #if pmod.enable == False:
        #    return

        if 'ignore' not in pmod.__dict__:
            pmod.ignore = False

        if 'iterate' not in pmod.__dict__:
            pmod.iterate = [ None ]

        if GlobalOptions.rtl == True and pmod.rtl == False:
            return

        if GlobalOptions.perf == True and pmod.perf == False:
            return

        if GlobalOptions.latency == True and pmod.perf == False:
            return

        if GlobalOptions.pps == True and pmod.perf == False:
            return

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

        nmods = None
        if GlobalOptions.modscale:
            nmods = GlobalOptions.modscale
        elif getattr(pmod, 'modscale', None):
            nmods = pmod.modscale

        if nmods:
            for s in range(nmods):
                newmod = copy.deepcopy(module)
                newmod.InitCopy(s+1)
                self.db[newmod.GID()] = newmod
        else:
            self.db[module.GID()] = module
        return

    def GetAll(self):
        if GlobalOptions.shuffle:
            modlist = list(self.db.values())
            random.shuffle(modlist)
            return modlist

        return sorted(self.db.values(), key=operator.attrgetter('runorder'))

    def Add(self, spec):
        self.__add(spec)
        return

ModuleStore = ModuleDatabase()
