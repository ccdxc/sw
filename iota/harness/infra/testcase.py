#! /usr/bin/python3
import pdb
import copy
import os
import subprocess
from iota.harness.infra.utils.logger import Logger as Logger

import iota.harness.infra.utils.timeprofiler as timeprofiler
import iota.harness.infra.types as types
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api

class VerifStep:
    def __init__(self, spec):
        self.__spec = spec
        self.__timer = timeprofiler.TimeProfiler()
        self.__resolve()
        return

    def __resolve(self):
        Logger.debug("Resolving testcase verif module: %s" % self.__spec.step)
        self.__mod = loader.Import(self.__spec.step, self.__spec.packages)
        return

    def __execute(self):
        return

    def Main(self):
        self.__timer.Start()
        self.__execute()
        self.__timer.Stop()
        return

    def PrintResultSummary(self):
        modname = "- %s" % self.__mod.__name__.split('.')[-1]
        print(types.FORMAT_TESTCASE_SUMMARY % (modname, "Pass", self.__timer.TotalTime()))
        return types.status.SUCCESS

class TestcaseDataIters:
    def __init__(self):
        self.__summary = None
        return

    def __add_to_summary(self, kv_summary):
        if self.__summary:
            self.__summary += kv_summary
        else:
            self.__summary = kv_summary
        return

    def AddKV(self, key, val):
        self.__add_to_summary("%s:%s," % (key, str(val)))
        return setattr(self, key, val)

    def Summary(self):
        return self.__summary

class TestcaseData:
    def __init__(self, dirname, args):
        self.__status = types.status.FAILURE
        self.__timer = timeprofiler.TimeProfiler()
        self.args = args
        self.iterators = TestcaseDataIters()
        self.__logs_dir = "%s/tcdata/%s/" % (api.GetTestsuiteLogsDir(), dirname)
        os.system("mkdir -p %s" % self.__logs_dir)
        return

    def SetStatus(self, status):
        self.__status = status
        return

    def GetStatus(self):
        return self.__status

    def GetLogsDir(self):
        return self.__logs_dir

    def StartTime(self):
        self.__timer.Start()
        return

    def StopTime(self):
        self.__timer.Stop()
        return

    def TotalTime(self):
        return self.__timer.TotalTime()

class Testcase:
    def __init__(self, spec):
        self.__spec = spec
        self.__tc = None
        self.__verifs = []
        self.__iterid = 0
        self.__resolve()

        self.__timer = timeprofiler.TimeProfiler()
        self.__iters = []
        self.__aborted = False
        self.status = types.status.UNAVAIL

        self.__setup_iters()

        self.__stats_pass = 0
        self.__stats_fail = 0
        self.__stats_ignored = 0
        self.__stats_error = 0
        return

    def __get_iter_directory(self, iter_id):
        return "%s_%d" % (self.__spec.name, iter_id)

    def __new_TestcaseData(self):
        self.__iterid += 1
        return TestcaseData(self.__get_iter_directory(self.__iterid), 
                            getattr(self.__spec, 'args', None))

    def __setup_simple_iters(self, spec):
        Logger.debug("Setting up simple iterators.")
        min_len = 0
        for k,v in spec.__dict__.items():
            if isinstance(v, list):
                iter_len = len(v)
                if min_len == 0 or min_len > iter_len: min_len = iter_len
                spec.__dict__[k] = iter(v)
            else:
                spec.__dict__[k] = None

        for inst in range(min_len):
            new_data = self.__new_TestcaseData()
            for k,v in spec.__dict__.items():
                if not v: continue
                new_data.iterators.AddKV(k, next(v))
                Logger.debug("- Adding K:%s V:" % k, v)
            self.__iters.append(new_data)
        return

    def __setup_nested_iters_mul(self, key, values, curr_data_list):
        new_data_list = []
        for val in values:
            if len(curr_data_list):
                for data in curr_data_list:
                    new_data = copy.deepcopy(data)
                    new_data.iterators.AddKV(key, val)
                    new_data_list.append(new_data)
            else:
                new_data = self.__new_TestcaseData()
                new_data.iterators.AddKV(key, val)
                new_data_list.append(new_data)
        return new_data_list

    def __setup_nested_iters(self, spec):
        data_list = []
        for k,v in spec.__dict__.items():
            if not isinstance(v, list): continue
            data_list = self.__setup_nested_iters_mul(k, v, data_list)
        self.__iters = data_list
        return

    def __setup_iters(self):
        spec = getattr(self.__spec, 'iterators', None)
        if spec is None:
            self.__iters.append(self.__new_TestcaseData())
            return
        iter_type = getattr(spec, 'type', None)
        if iter_type and iter_type == 'simple':
            return self.__setup_simple_iters(spec)
        elif iter_type and iter_type == 'nested':
            return self.__setup_nested_iters(spec)
        else:
            api.Logger.error("Invalid Iterator Type: %s" % iter_type)
            assert(0)
        return

    def __resolve_testcase(self):
        Logger.debug("Resolving testcase module: %s" % self.__spec.testcase)
        self.__tc = loader.Import(self.__spec.testcase, self.__spec.packages)
        verifs_spec = getattr(self.__spec, 'verifs', [])
        if verifs_spec is None:
            return types.status.SUCCESS
        for v in verifs_spec:
            v.packages = self.__spec.packages
            verif = VerifStep(v)
            self.__verifs.append(verif)
        return types.status.SUCCESS

    def __resolve(self):
        ret = self.__resolve_testcase()
        if ret != types.status.SUCCESS:
            return ret
        return types.status.SUCCESS

    def __mk_testcase_directory(self, newdir):
        Logger.info("Creating Testcase directory: %s" % newdir)
        command = "mkdir -p %s && chmod 777 %s" % (newdir, newdir)
        req = api.Trigger_CreateExecuteCommandsRequest()
        for nodename in api.GetNaplesHostnames():
            api.Trigger_AddHostCommand(req, nodename, command)
        for wl in api.GetWorkloads():
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, command)
        resp = api.Trigger(req)
        if not api.Trigger_IsSuccess(resp):
            Logger.error("Failed to create destination directory %s" % newdir)
            return types.status.FAILURE
        return types.status.SUCCESS

    def __update_stats(self, status):
        if status == types.status.SUCCESS:
            self.__stats_pass += 1
        elif status == types.status.FAILURE:
            self.__stats_fail += 1
        elif status == types.status.IGNORED:
            self.__stats_ignored += 1
        else:
            self.__stats_error += 1
        return

    def __execute(self):
        ignored = getattr(self.__spec, "ignore", False)
        
        final_result = types.status.SUCCESS
        iter_id = 1
        for iter_data in self.__iters:
            iter_data.StartTime()

            api.ChangeDirectory("")
            Logger.SetTestcaseID(iter_id)
            
            tc_iter_directory = self.__get_iter_directory(iter_id)
            Logger.debug("Testcase Iteration directory = %s", tc_iter_directory)
            ret = self.__mk_testcase_directory(tc_iter_directory)
            if ret != types.status.SUCCESS: 
                self.__update_stats(ret)
                return ret
            
            api.ChangeDirectory(tc_iter_directory)
            
            result = types.status.SUCCESS
            setup_result = loader.RunCallback(self.__tc, 'Setup', False, iter_data)
            if setup_result != types.status.SUCCESS:
                Logger.error("Setup callback failed, Cannot continue, switching to Teardown")
                loader.RunCallback(self.__tc, 'Teardown', False, iter_data)
                result = setup_result
            else:
                trigger_result = loader.RunCallback(self.__tc, 'Trigger', True, iter_data)
                if trigger_result != types.status.SUCCESS:
                    result = trigger_result

                verify_result = loader.RunCallback(self.__tc, 'Verify', True, iter_data)
                if verify_result != types.status.SUCCESS:
                    result = verify_result

                teardown_result = loader.RunCallback(self.__tc, 'Teardown', False, iter_data)
                if teardown_result != types.status.SUCCESS:
                    Logger.error("Teardown callback failed.")
                    result = teardown_result

                iter_data.SetStatus(result)
                iter_data.StopTime()

                if self.__aborted:
                    iter_data.SetStatus(types.status.ABORTED)
                    self.__update_stats(types.status.ABORTED)
                    return types.status.FAILURE
 
            if result != types.status.SUCCESS:
                if ignored:
                    iter_data.SetStatus(types.status.IGNORED)
                else:
                    final_result = result
            iter_id = iter_id + 1

        api.ChangeDirectory("")
        return final_result

    def __get_owner(self):
        result = subprocess.run([ "git", "log", "-n", "1", "--format=%an",
                                  "%s" % self.__tc.__file__], stdout=subprocess.PIPE)
        owner = result.stdout.decode('utf-8')
        owner = owner.replace('\n', '').split(' ')[0]
        
        return owner

    def PrintResultSummary(self):
        for iter_data in self.__iters:
            iters_str = iter_data.iterators.Summary()
            print(types.FORMAT_TESTCASE_SUMMARY %\
                  (self.__spec.name, self.__get_owner(),
                   types.status.str(iter_data.GetStatus()).title(), 
                   iter_data.TotalTime()))
            if iters_str: print("- Iterators: %s" % iters_str)
            for v in self.__verifs:
                v.PrintResultSummary()

    def Name(self):
        return self.__spec.name

    def Abort(self):
        self.__aborted = True
        return

    
    def __aggregate_stats(self):
        for iter_data in self.__iters:
            self.__update_stats(iter_data.GetStatus())
        return

    def GetStats(self):
        return (self.__stats_pass, self.__stats_fail, self.__stats_ignored, self.__stats_error)

    def Main(self):
        Logger.SetTestcase(self.Name())
        Logger.info("Starting Testcase: %s" % self.Name())
        self.__timer.Start()
        self.status = self.__execute()
        self.__aggregate_stats()
        self.__timer.Stop()
        Logger.info("Testcase %s FINAL RESULT = %d" % (self.Name(), self.status))
        return self.status
