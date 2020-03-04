#! /usr/bin/python3
import atexit
import json
import os
import pdb
import re
import time
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
import iota.harness.infra.core_collector as core_collector
import iota.harness.infra.utils.utils as utils
import iota.harness.infra.utils.timeprofiler as timeprofiler

from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions


class TestSuiteResults(object):
    def __init__(self, testbed=None, repo=None, sha=None, shaTitle=None, targetId=None):
        #these var names map directly to test results viewer schema
        #do not change.
        if testbed:
            self.Testbed = testbed
        else:
            self.Testbed = os.getenv("TESTBED_ID","jobd")
        if repo:
            self.Repository = repo
        else:
            self.Repository = os.getenv("JOB_REPOSITORY")
        if repo:
            self.SHA = sha
        else:
            self.SHA = os.getenv("")
        if shaTitle:
            self.SHATitle = shaTitle
        else:
            self.SHATitle = os.getenv("")
        if targetId:
            self.TargetID = targetId
        else:
            self.TargetID = os.getenv("TARGET_ID")
        self.Testcases = []

    def addTestcase(self, tc):
        self.Testcases.append(tc)

    def getTestCaseResults(self):
        return self.Testcases


class TestSuite:
    def __init__(self, spec):
        self.__spec = spec
        self.__testbundles = []
        self.__verifs = []
        self.__debugs = []
        self.__setups = []
        self.__setup_complete = False

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
        self.__skip = self.__apply_skip_filters()
        self.__ignoreList = getattr(spec.meta, "ignore_list", [])
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

    def GetFirmwareType(self):
        if hasattr(self._TestSuite__spec.meta,"firmware"):
            fmw=self._TestSuite__spec.meta.firmware.lower()
            if fmw == "gold":
                return types.firmware.GOLD
            elif fmw == "main":
                return types.firmware.MAIN
            else:
                raise ValueError("firmware must be gold or main. user specified: {0}".format(fmw))

    def GetPackages(self):
        return self.__spec.packages

    def SetupComplete(self):
        return self.__setup_complete

    def GetImages(self):
        return self.__spec.images

    def GetTopology(self):
        return self.__topology

    def GetNicMode(self):
        return self.__spec.meta.nicmode

    def GetVerifs(self):
        return self.__verifs

    def GetDebugs(self):
        return self.__debugs

    def GetSetups(self):
        return self.__setups

    def GetPacakges(self):
        return self.__spec.packages

    def IsConfigOnly(self):
       return getattr(self.__spec.meta, "cfgonly", False)

    def IsBitw(self):
       return getattr(self.__spec.meta, "nicmode", None) == "bitw"

    def GetFwdMode(self):
       return getattr(self.__spec.meta, "fwdmode", "")

    def GetPolicyMode(self):
       return getattr(self.__spec.meta, "policymode", "")

    def DoConfig(self):
        return self.__setup_config()

    def __resolve_calls(self):
        if getattr(self.__spec, 'common', None) and getattr(self.__spec.common, 'verifs', None):
            self.__verifs = self.__spec.common.verifs
        if getattr(self.__spec, 'common', None) and getattr(self.__spec.common, 'debugs', None):
            self.__debugs = self.__spec.common.debugs
        if getattr(self.__spec, 'common', None) and getattr(self.__spec.common, 'setups', None):
            self.__setups = self.__spec.common.setups

    def __import_testbundles(self):
        if not GlobalOptions.skip_sanity:
            for bunfile in self.__spec.testbundles.sanity:
                tbun = testbundle.TestBundle(bunfile, self)
                self.__testbundles.append(tbun)

        if GlobalOptions.extended:
            for bunfile in getattr(self.__spec.testbundles, 'extended', []):
                tbun = testbundle.TestBundle(bunfile, self)
                self.__testbundles.append(tbun)

        if GlobalOptions.regression:
            for bunfile in getattr(self.__spec.testbundles, 'regression', []):
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
            s.name = s.step
            args = getattr(s, "args", None)
            s.step = loader.Import(s.step, self.__spec.packages)
            s.args = getattr(s, "args", None)
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
            # Reset the running directory before every step
            Logger.info("Starting Config Step: ", s.step)
            api.ChangeDirectory(None)
            status = loader.RunCallback(s.step, 'Main', True, getattr(s, "args", None))
            if status != types.status.SUCCESS:
                Logger.error("ERROR: Failed to run config step", s.step)
                return status
        return types.status.SUCCESS


    def __setup(self):
        ret = None

        if GlobalOptions.skip_setup:
            ret = self.__topology.Build(self)
            if ret != types.status.SUCCESS:
                return ret
        else:
            ret = self.__topology.Setup(self)
            if ret != types.status.SUCCESS:
                return ret
        ret = self.__setup_config()
        if ret != types.status.SUCCESS:
            return ret

        self.__setup_complete = True
        return types.status.SUCCESS

    def __invoke_teardown(self):
        teardown_spec = getattr(self.__spec, 'teardown', [])
        if teardown_spec is None:
            return types.status.SUCCESS
        for s in teardown_spec:
            Logger.info("Starting Teardown Step: ", s.step)
            status = loader.RunCallback(s.step, 'Main', True, getattr(s, "args", None))
            if status != types.status.SUCCESS:
                Logger.error("ERROR: Failed to run teardown step", s.step)
                return status
        return types.status.SUCCESS

    def __teardown(self):
        return self.__invoke_teardown()

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
        if self.__stats_target != 0:
            self.__stats_target = max(self.__stats_target, self.__stats_total)
        return

    def __execute_testbundles(self):
        result = types.status.SUCCESS
        for tbun in self.__testbundles:
            tbun.setIgnoreList(self.__ignoreList)
            self.__curr_tbun = tbun
            ret = tbun.Main()
            #Don't propogate disabled status up.
            if ret == types.status.DISABLED:
                ret = types.status.SUCCESS
            elif ret != types.status.SUCCESS:
                result = ret
                if result == types.status.CRITICAL and GlobalOptions.stop_on_critical:
                    return ret
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

    def __apply_skip_filters(self):
        if GlobalOptions.testsuites and self.Name() not in GlobalOptions.testsuites:
            Logger.debug("Skipping Testsuite: %s because of command-line filters." % self.Name())
            return True

        #if self.GetTestbedType() != types.tbtype.ANY and\
        #   self.GetTestbedType() != store.GetTestbed().GetTestbedType()\
        #   and not GlobalOptions.dryrun:
        #    Logger.debug("Skipping Testsuite: %s due to testbed type mismatch." % self.Name())
        #   return True

        if not store.GetTestbed().IsSimulation() and  not store.GetTestbed().GetOs().intersection(self.__get_oss()) and not GlobalOptions.dryrun:
            Logger.debug("Skipping Testsuite: %s due to OS mismatch." % self.Name())
            return True

        enable = getattr(self.__spec.meta, 'enable', True)
        return not enable

    def IsSkipped(self):
        return self.__skip

    def CollectCores(self):
        try:
            destCoreDir = "corefiles/{0}".format(re.sub('[\W]+','_',self.Name()))
            core_collector.CollectCores(GlobalOptions.testbed_json, destCoreDir, "vm", "vm")
        except:
            Logger.debug("failed to collect cores. error was {0}".format(traceback.format_exc()))

    def writeTestResults(self):
        filename = "testsuite_{0}_results.json".format(self.Name())
        try:
            tsr = TestSuiteResults()
            for tbun in self.__testbundles:
                if tbun.getTestBundleResults():
                    tsr.Testcases.extend(tbun.getTestBundleResults())
            with open(filename,'w') as outfile:
                json.dump(tsr, outfile, default=lambda x: x.__dict__, sort_keys=True)
        except:
            Logger.debug("failed to save test results to file {0}. error was: {1}".format(filename,traceback.format_exc()))

    def Main(self):
        if self.__skip:
            Logger.debug("Skipping Testsuite: %s due to filters." % self.Name())
            return types.status.SUCCESS

        atexit.register(logcollector.CollectLogs)
        atexit.register(logcollector.CollectTechSupport,(self.Name()))
        atexit.register(self.CollectCores)
        atexit.register(self.writeTestResults)

        # Start the testsuite timer
        self.__timer.Start()

        # Update logger
        Logger.SetTestsuite(self.Name())
        Logger.info("Starting Testsuite: %s" % self.Name())
        Logger.info("Testsuite {0} timestamp: {1}".format(self.Name(), time.asctime()))

        if self.GetFirmwareType() == types.firmware.GOLD:
            Logger.debug("setting global firmware type to gold")
            GlobalOptions.use_gold_firmware = True

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
        self.__resolve_calls()

        status = self.__setup()
        if status != types.status.SUCCESS:
            self.__timer.Stop()
            return status

        self.result = self.__execute_testbundles()
        self.__update_stats()
        Logger.info("Testsuite %s FINAL STATUS = %d" % (self.Name(), self.result))

        status = self.__teardown()
        # TODO: add teardown validation
        #moved to atexit() call
        #logcollector.CollectLogs()
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
              ("Testbundle", "Pass", "Fail", "Ignore", "Dis/Err", "Total", "Target", "%Done", "Result", "Duration"))
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
        if attr in self.__attrs:
            return self.__attrs[attr]
        return None
