#! /usr/bin/python3
import pdb
import copy
import os
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
    def __init__(self, name, args):
        self.__status = types.result.PASS
        self.args = args
        self.name = name
        self.iterators = TestcaseDataIters()
        return
    def SetStatus(self, status):
        self.__status = status
        return
    def GetStatus(self):
        return self.__status
    def Name(self):
        return self.name

    def _setup_tc_data(self, tc_id):
        self.tc_id = tc_id
        self.data_dir = api.GetTestDataDir() + "/" + self.name + "/" + self.GetTcID()
        os.system("mkdir -p %s" % self.data_dir)

    def GetTcID(self):
        return self.name + "_" + str(self.tc_id)
    def GetTcDir(self):
        return self.data_dir

class Testcase:
    def __init__(self, spec):
        self.__spec = spec
        self.__tc = None
        self.__verifs = []
        self.__resolve()

        self.__timer = timeprofiler.TimeProfiler()
        self.__iters = []
        self.__aborted = False
        self.status = types.status.UNAVAIL

        self.__setup_iters()
        return

    def __new_TestcaseData(self):
        return TestcaseData(self.Name(), getattr(self.__spec, 'args', None))

    def __setup_simple_iters(self, spec):
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

    def __execute(self):
        final_result = types.status.SUCCESS
        iter_id = 1
        for iter_data in self.__iters:
            iter_data._setup_tc_data(iter_id)
            Logger.SetTestcaseID(iter_id)
            iter_id = iter_id + 1
            result = types.status.SUCCESS
            setup_result = loader.RunCallback(self.__tc, 'Setup', False, iter_data)
            if setup_result != types.status.SUCCESS:
                Logger.error("Setup callback failed, Cannot continue, switching to Teardown")
                loader.RunCallback(self.__tc, 'Teardown', False, iter_data)
                result = setup_result
                continue

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

            if self.__aborted:
                return types.status.FAILURE

            if result != types.status.SUCCESS and not getattr(self.__spec, "ignore", False):
                final_result = result

        return final_result

    def PrintResultSummary(self):
        for iter_data in self.__iters:
            print(types.FORMAT_TESTCASE_SUMMARY %\
                  (self.__spec.name, types.status.str(iter_data.GetStatus()).title(), self.__timer.TotalTime()))
            summary = iter_data.iterators.Summary()
            if summary: print("- Iterators: %s" % summary)
            for v in self.__verifs:
                v.PrintResultSummary()

    def Name(self):
        return self.__spec.name

    def Abort(self):
        self.__aborted = True
        return

    def Main(self):
        Logger.SetTestcase(self.Name())
        Logger.info("Starting Testcase: %s" % self.Name())
        self.__timer.Start()
        self.status = self.__execute()
        self.__timer.Stop()
        Logger.info("Testcase %s FINAL RESULT = %d" % (self.Name(), self.status))
        return self.status
