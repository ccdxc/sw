#! /usr/bin/python3
import pdb
import traceback

import iota.harness.infra.store as store
import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.testcase as testcase
import iota.harness.infra.topology as topology
import iota.harness.infra.logcollector as logcollector
import iota.harness.infra.utils.utils as utils
import iota.harness.infra.utils.timeprofiler as timeprofiler

from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

class TestBundle:
    def __init__(self, bunfile, parent):
        self.__bunfile = bunfile
        self.__parent = parent
        self.__spec = None
        self.__testcases = []
        self.__skip = False
        self.__aborted = False
        self.__timer = timeprofiler.TimeProfiler()

        self.__stats_pass = 0
        self.__stats_fail = 0
        self.__stats_ignored = 0
        self.__stats_error = 0
        self.__stats_total = 0
        self.__stats_target = 0
        self.__load_bundle()
        return

    def Name(self):
        return self.__spec.meta.name

    def __read_spec(self, fullpath):
        spec = parser.YmlParse(fullpath)
        spec.meta.os = getattr(spec.meta, 'os', [ 'linux' ])
        spec.meta.nics = getattr(spec.meta, 'nics', [])
        return spec

    def __apply_skip_filters(self):
        if store.GetTestbed().GetOs() not in self.__spec.meta.os and not GlobalOptions.dryrun:
            Logger.debug("Skipping Testbundle: %s due to OS mismatch." % self.__spec.meta.name)
            return True
        if GlobalOptions.testbundles and self.__spec.meta.name not in  GlobalOptions.testbundles:
            Logger.info("Skipping Testbundle: %s due to cmdline filter." % self.__spec.meta.name)
            return True
        if store.GetTopology().ValidateNics(self.__spec.meta.nics) != True and not GlobalOptions.dryrun:
            Logger.debug("Skipping Testbundle: %s due to Incompatible NICs." % self.__spec.meta.name)
            return True
        return False

    def __load_bundle(self):
        fullpath = "%s/iota/test/iris/testbundles/%s" % (api.GetTopDir(), self.__bunfile)
        Logger.debug("Importing Testbundle %s" % fullpath)
        self.__spec = self.__read_spec(fullpath)
        return

    def __resolve_testcases(self):
        for tc_spec in self.__spec.testcases:
            if getattr(tc_spec, 'disable', False):
                Logger.info("Skipping disabled test case %s" % tc_spec.name)
                continue
            tc_spec.packages = self.__parent.GetPackages()
            if getattr(self.__spec, 'common', None) and getattr(self.__spec.common, 'verifs', None):
                tc_spec.verifs = self.__spec.common.verifs
            tc = testcase.Testcase(tc_spec, self)
            self.__testcases.append(tc)
        return types.status.SUCCESS

    def PrintReport(self):
        if self.__skip:
           return types.status.SUCCESS
        for tc in self.__testcases:
            tc.PrintResultSummary()
        return

    def PrintSummary(self):
        if self.__skip:
           return types.status.SUCCESS
        if self.__stats_target:
            pc_complete = "%02.02f" % (float(self.__stats_total)/float(self.__stats_target)*100)
        else:
            pc_complete = 'NA'
        print(types.FORMAT_ALL_TESTBUNDLE_SUMMARY %\
              (self.__spec.meta.name, self.__stats_pass, self.__stats_fail, self.__stats_ignored,
               self.__stats_error, self.__stats_total, self.__stats_target, pc_complete,
               types.status.str(self.result).title(), self.__timer.TotalTime()))
        return types.status.SUCCESS

    def __execute_testcases(self):
        result = types.status.SUCCESS
        for tc in self.__testcases:
            ret = tc.Main()
            if ret != types.status.SUCCESS:
                result = ret
                if GlobalOptions.no_keep_going:
                    return ret
            if self.__aborted:
                return types.status.FAILURE
        return result

    def __update_stats(self):
        for tc in self.__testcases:
            p,f,i,e = tc.GetStats()
            self.__stats_pass += p
            self.__stats_fail += f
            self.__stats_ignored += i
            self.__stats_error += e
            
        self.__stats_total = (self.__stats_pass + self.__stats_fail +\
                              self.__stats_ignored + self.__stats_error)
        self.__stats_target = getattr(self.__spec.meta, 'targetcount', 0)
        return

    def Abort(self):
        self.__aborted = True
        return

    def GetStats(self):
        return (self.__stats_pass, self.__stats_fail, self.__stats_ignored, self.__stats_error, self.__stats_target)

    def Main(self):
        self.__skip = self.__apply_skip_filters()
        if self.__skip: 
            return types.status.SUCCESS

        # Start the testsuite timer
        self.__timer.Start()
        self.__resolve_testcases()

        # Update logger
        Logger.SetTestbundle(self.Name())
        Logger.info("Starting Testbundle: %s" % self.Name())

        self.result = self.__execute_testcases()
        self.__update_stats()
        Logger.info("Testbundle %s FINAL STATUS = %d" % (self.Name(), self.result))
        
        logcollector.CollectLogs()
        self.__timer.Stop()
        return self.result
