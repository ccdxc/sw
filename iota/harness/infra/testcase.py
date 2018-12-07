#! /usr/bin/python3
import pdb
import copy
import os
import subprocess

import iota.harness.infra.utils.timeprofiler as timeprofiler
import iota.harness.infra.types as types
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.utils.utils as utils

from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
def get_owner(file):
    result = subprocess.run([ "git", "log", "-n", "1", "--format=%an",
			  "%s" % file], stdout=subprocess.PIPE)
    owner = result.stdout.decode('utf-8')
    owner = owner.replace('\n', '').split(' ')[0]
    return owner

class VerifStep:
    def __init__(self, spec):
        self.__spec = spec
        self.__timer = timeprofiler.TimeProfiler()
        self.__resolve()
        self.__status = api.types.status.FAILURE
        return

    def __resolve(self):
        Logger.debug("Resolving testcase verif module: %s" % self.__spec.step)
        self.__mod = loader.Import(self.__spec.step, self.__spec.packages)
        return

    def __execute(self):
        Logger.debug("Running testcase verif module: %s" % self.__spec.step)
        return loader.RunCallback(self.__mod, 'Main', True, None) 

    def Main(self):
        self.__timer.Start()
        self.__status = self.__execute()
        self.__timer.Stop()
        if self.__status != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return self.__status

    def __get_owner(self):
        return get_owner(self.__mod.__file__)

    def PrintResultSummary(self):
        modname = "- %s" % self.__mod.__name__.split('.')[-1]
        print(types.FORMAT_TESTCASE_SUMMARY % (modname, self.__get_owner(), types.status.str(self.__status).title(), self.__timer.TotalTime()))
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
    def __init__(self, instid, args):
        self.__status = types.status.FAILURE
        self.__timer = timeprofiler.TimeProfiler()
        self.args = args
        self.iterators = TestcaseDataIters()
        self.SetInstanceId(instid)
        return

    def SetInstanceId(self, instid):
        self.__instid = instid
        self.__logs_dir = "%s/tcdata/%s/" % (api.GetTestsuiteLogsDir(), instid)
        os.system("mkdir -p %s" % self.__logs_dir)
        return

    def GetInstanceId(self):
        return self.__instid

    def GetInst(self):
        return self.__instid

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

    def Name(self):
        return self.__instid

class Testcase:
    def __init__(self, spec, parent):
        self.__spec = spec
        self.__parent = parent
        self.__spec.name = self.__spec.name.replace(' ', '_')
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

        self.__enable = True
        return

    def __get_instance_id(self, iter_id):
        return "%s_%d" % (self.__spec.name, iter_id)

    def __new_TestcaseData(self, src = None):
        self.__iterid += 1
        if src is None:
            new = TestcaseData(self.__get_instance_id(self.__iterid), 
                               getattr(self.__spec, 'args', None))
        else:
            new = copy.deepcopy(src)
            new.SetInstanceId(self.__get_instance_id(self.__iterid))
        return new

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
                    new_data = self.__new_TestcaseData(data)
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
        Logger.debug("Creating Testcase directory: %s" % newdir)
        command = "mkdir -p %s && chmod 777 %s" % (newdir, newdir)
        req = api.Trigger_CreateExecuteCommandsRequest()
        for nodename in api.GetWorkloadNodeHostnames():
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

    def __run_common_verifs(self):
        result = types.status.SUCCESS
        for s in self.__verifs:
            status = s.Main()
            if status != types.status.SUCCESS:
                result = status
        return result


    def __execute(self):
        ignored = getattr(self.__spec, "ignore", False)
        
        final_result = types.status.SUCCESS
        for iter_data in self.__iters:
            iter_data.StartTime()

            api.ChangeDirectory("")
            Logger.SetTestcase(iter_data.GetInstanceId())
            Logger.debug("Testcase Iteration directory = %s" % iter_data.GetInstanceId())
            ret = self.__mk_testcase_directory(iter_data.GetInstanceId())
            if ret != types.status.SUCCESS: 
                iter_data.SetStatus(ret)
                return ret
            
            api.ChangeDirectory(iter_data.GetInstanceId())
            
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

                    verify_result = self.__run_common_verifs();
                    if verify_result != types.status.SUCCESS:
                        Logger.error("Common verifs failed.")
                        result = verify_result

                    teardown_result = loader.RunCallback(self.__tc, 'Teardown', False, iter_data)
                    if teardown_result != types.status.SUCCESS:
                        Logger.error("Teardown callback failed.")
                        result = teardown_result

                    iter_data.SetStatus(result)
                    iter_data.StopTime()

                    if self.__aborted:
                        iter_data.SetStatus(types.status.ABORTED)
                        return types.status.FAILURE

                    if result != types.status.SUCCESS and GlobalOptions.no_keep_going:
                        Logger.error("Error: STOPPING ON FIRST FAILURE.")
                        iter_data.SetStatus(result)
                        return ret
 
            if result != types.status.SUCCESS:
                if ignored:
                    iter_data.SetStatus(types.status.IGNORED)
                else:
                    final_result = result

        api.ChangeDirectory("")
        return final_result

    def __get_owner(self):
        return get_owner(self.__tc.__file__)
        

    def PrintResultSummary(self):
        if not self.__enable: return

        for iter_data in self.__iters:
            iters_str = iter_data.iterators.Summary()
            print(types.FORMAT_TESTCASE_SUMMARY %\
                  (self.__parent.Name(), iter_data.Name(), self.__get_owner(),
                   types.status.str(iter_data.GetStatus()).title(), 
                   iter_data.TotalTime()))
            if iters_str: print("- Iterators: %s" % iters_str)
            for v in self.__verifs:
                v.PrintResultSummary()
        return

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
        if self.__enable:
            self.__aggregate_stats()
        return (self.__stats_pass, self.__stats_fail, self.__stats_ignored, self.__stats_error)

    def Main(self):
        if GlobalOptions.testcases and self.Name() not in GlobalOptions.testcases:
            Logger.info("Skipping Testcase: %s due to cmdline filter." % self.Name())
            self.__enable = False
            return types.status.SUCCESS

        Logger.SetTestcase(self.Name())
        Logger.info("Starting Testcase: %s" % self.Name())
        self.__timer.Start()
        try:
            self.status = self.__execute()
        except:
            utils.LogException(Logger)
            Logger.error("EXCEPTION: Aborting Testcase Execution.")
            self.status = types.status.ERROR
        self.__timer.Stop()
        Logger.info("Testcase %s FINAL RESULT = %d" % (self.Name(), self.status))
        return self.status
