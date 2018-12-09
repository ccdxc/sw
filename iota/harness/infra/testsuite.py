#! /usr/bin/python3
import pdb
import traceback

import iota.harness.infra.store as store
import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.testbundle as testbundle
import iota.harness.infra.testcase as testcase
import iota.harness.infra.topology as topology
import iota.harness.infra.logcollector as logcollector
import iota.harness.infra.utils.utils as utils
import iota.harness.infra.utils.timeprofiler as timeprofiler

from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

class TestSuite:
    def __init__(self, spec):
        self.__spec = spec
        self.__testbundles = []

        self.__aborted = False
        self.__attrs = {}
        self.__timer = timeprofiler.TimeProfiler()

        self.__stats_pass = 0
        self.__stats_fail = 0
        self.__stats_ignored = 0
        self.__stats_error = 0
        self.__stats_total = 0
        self.__stats_target = 0
        self.result = types.status.FAILURE
        return

    def Abort(self):
        self.__aborted = True
        self.__curr_tbun.Abort()
        return

    def GetTestbedType(self):
        if self.__spec.meta.mode.lower() == 'hardware':
            return types.tbtype.HARDWARE
        elif self.__spec.meta.mode.lower() == 'simulation':
            return types.tbtype.SIMULATION
        elif self.__spec.meta.mode.lower() == 'hybrid':
            return types.tbtype.HYBRID
        return types.tbtype.ANY

    def GetPackages(self):
        return self.__spec.packages 

    def GetImages(self):
        return self.__spec.images

    def GetTopology(self):
        return self.__topology

    def GetNicMode(self):
        return self.__spec.meta.nicmode

    def IsConfigOnly(self):
       return getattr(self.__spec.meta, "cfgonly", False)
 
    def DoConfig(self):
        return self.__setup_config()

    def __import_testbundles(self):
        for bunfile in self.__spec.testbundles:
            tbun = testbundle.TestBundle(bunfile, self)
            self.__testbundles.append(tbun)
        return

    def __resolve_teardown(self):
        teardown_spec = getattr(self.__spec, 'teardown', [])
        if teardown_spec is None:
            return types.status.SUCCESS
        for s in self.__spec.teardown:
            Logger.debug("Resolving teardown module: %s" % s.step)
            s.step = loader.Import(s.step, self.__spec.packages)
        return types.status.SUCCESS

    def __expand_iterators(self):
        return

    def __parse_setup_topology(self):
        topospec = getattr(self.__spec.setup, 'topology', None)
        if not topospec:
            Logger.error("Error: No topology specified in the testsuite.")
            assert(0)
        self.__topology = topology.Topology(topospec)
        store.SetTopology(self.__topology)
        return types.status.SUCCESS

    def __resolve_setup_config(self):
        cfgspec = getattr(self.__spec.setup, 'config', None)
        if not cfgspec:
            return types.status.SUCCESS
        for s in self.__spec.setup.config:
            Logger.debug("Resolving config step: %s" % s.step)
            s.step = loader.Import(s.step, self.__spec.packages)
        return types.status.SUCCESS

    def __parse_setup(self):
        ret = self.__parse_setup_topology()
        if ret != types.status.SUCCESS:
            return ret

        ret = self.__resolve_setup_config()
        if ret != types.status.SUCCESS:
            return ret
        return types.status.SUCCESS

    def __setup_config(self):
        for s in self.__spec.setup.config:
            status = loader.RunCallback(s.step, 'Main', True, None)
            if status != types.status.SUCCESS:
                return status
        return types.status.SUCCESS

    def __setup(self):
        ret = self.__topology.Setup(self)
        if ret != types.status.SUCCESS:
            return ret
        ret = self.__setup_config()
        if ret != types.status.SUCCESS:
            return ret

        return types.status.SUCCESS

    def __update_stats(self):
        for tbun in self.__testbundles:
            p,f,i,e,t = tbun.GetStats()
            self.__stats_pass += p
            self.__stats_fail += f
            self.__stats_ignored += i
            self.__stats_error += e
            self.__stats_target += t
            
        self.__stats_total = (self.__stats_pass + self.__stats_fail +\
                              self.__stats_ignored + self.__stats_error)
        return

    def __execute_testbundles(self):
        result = types.status.SUCCESS
        for tbun in self.__testbundles:
            self.__curr_tbun = tbun
            ret = tbun.Main()
            if ret != types.status.SUCCESS:
                result = ret
                if GlobalOptions.no_keep_going:
                    return ret
            if self.__aborted:
                return types.status.FAILURE
        return result

    def Name(self):
        return self.__spec.meta.name

    def Mode(self):
        return self.__spec.meta.mode

    def LogsDir(self):
        return "%s/iota/logs/%s" % (api.GetTopDir(), self.Name())

    def __get_oss(self):
        return getattr(self.__spec.meta, 'os', ['linux'])

    def __is_regression(self):
        return getattr(self.__spec.meta, 'type', None) == 'regression'

    def __apply_skip_filters(self):
        if GlobalOptions.testsuites and self.Name() not in GlobalOptions.testsuites:
            Logger.debug("Skipping Testsuite: %s because of command-line filters." % self.Name())
            return True

        if self.GetTestbedType() != types.tbtype.ANY and\
           self.GetTestbedType() != store.GetTestbed().GetTestbedType()\
           and not GlobalOptions.dryrun:
            Logger.debug("Skipping Testsuite: %s due to testbed type mismatch." % self.Name())
            return True

        if GlobalOptions.regression and not self.__is_regression():
            Logger.debug("Skipping non regression testsuite : %s" % self.Name())
            return True

        if not GlobalOptions.regression and self.__is_regression():
            Logger.debug("Skipping regression testsuite : %s" % self.Name())
            return True

        if store.GetTestbed().GetOs() not in self.__get_oss() and not GlobalOptions.dryrun:
            Logger.debug("Skipping Testsuite: %s due to OS mismatch." % self.Name())
            return True
       
        enable = getattr(self.__spec.meta, 'enable', True)
        return not enable

    def Main(self):
        self.__skip = self.__apply_skip_filters()
        if self.__skip:
            Logger.debug("Skipping Testsuite: %s due to filters." % self.Name())
            return types.status.SUCCESS

        # Start the testsuite timer
        self.__timer.Start()
        
        # Update logger
        Logger.SetTestsuite(self.Name())
        Logger.info("Starting Testsuite: %s" % self.Name())

        # Initialize Testbed for this testsuite
        status = store.GetTestbed().InitForTestsuite(self)
        if status != types.status.SUCCESS:
            self.__timer.Stop()
            return status
 
        status = self.__parse_setup()
        if status != types.status.SUCCESS:
            self.__timer.Stop()
            return status
       
        self.__import_testbundles()
        self.__resolve_teardown()
        self.__expand_iterators()

        status = self.__setup()
        if status != types.status.SUCCESS:
            self.__timer.Stop()
            return status
    
        self.result = self.__execute_testbundles()
        self.__update_stats()
        Logger.info("Testsuite %s FINAL STATUS = %d" % (self.Name(), self.result))
        
        logcollector.CollectLogs()
        self.__timer.Stop()
        return self.result

    def PrintReport(self):
        if self.__skip:
           return types.status.SUCCESS
        print("\nTestSuite: %s" % self.__spec.meta.name)
        print(types.HEADER_SUMMARY)
        print(types.FORMAT_TESTCASE_SUMMARY %\
              ("Testbundle", "Testcase", "Owner", "Result", "Duration"))
        print(types.HEADER_SUMMARY)
        for tbun in self.__testbundles:
            tbun.PrintReport()
        print(types.HEADER_SUMMARY)
        return

    def PrintBundleSummary(self):
        if self.__skip:
           return types.status.SUCCESS
        print("\nTestBundle Summary for TestSuite: %s" % self.__spec.meta.name)
        print(types.HEADER_SHORT_SUMMARY)
        print(types.FORMAT_ALL_TESTSUITE_SUMMARY %\
              ("Testbundle", "Pass", "Fail", "Ignore", "Error", "Total", "Target", "%Done", "Result", "Duration"))
        print(types.HEADER_SHORT_SUMMARY)
        for tbun in self.__testbundles:
            tbun.PrintSummary()
        print(types.HEADER_SHORT_SUMMARY)
        return

    def PrintSummary(self):
        if self.__skip:
           return types.status.SUCCESS
        if self.__stats_target:
            pc_complete = "%02.02f" % (float(self.__stats_total)/float(self.__stats_target)*100)
        else:
            pc_complete = 'NA'
        print(types.FORMAT_ALL_TESTSUITE_SUMMARY %\
              (self.__spec.meta.name, self.__stats_pass, self.__stats_fail, self.__stats_ignored,
               self.__stats_error, self.__stats_total, self.__stats_target, pc_complete,
               types.status.str(self.result).title(), self.__timer.TotalTime()))
        return types.status.SUCCESS

    def SetAttr(self, attr, value):
        Logger.info("Adding new Testsuite attribute: %s = " % attr, value)
        self.__attrs[attr] = value
        return

    def GetAttr(self, attr):
        return self.__attrs[attr]
