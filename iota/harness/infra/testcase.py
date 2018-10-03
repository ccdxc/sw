#! /usr/bin/python3
import pdb
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
        
class TestcaseData:
    def __init__(self):
        return

class Testcase:
    def __init__(self, spec):
        self.__spec = spec
        self.__tc = None
        self.__verifs = []
        self.__resolve()

        self.__timer = timeprofiler.TimeProfiler()
        self.__data = TestcaseData()
        return

    def __resolve_testcase(self):
        Logger.debug("Resolving testcase module: %s" % self.__spec.testcase)
        self.__tc = loader.Import(self.__spec.testcase, self.__spec.packages)
        verifs_spec = getattr(self.__spec, 'verifs', [])
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
        loader.RunCallback(self.__tc, 'Setup', False, self.__data)
        loader.RunCallback(self.__tc, 'Trigger', True, self.__data)
        result = loader.RunCallback(self.__tc, 'Verify', True, self.__data)
        loader.RunCallback(self.__tc, 'Teardown', False, self.__data)
        return result

    def PrintResultSummary(self):
        print(types.FORMAT_TESTCASE_SUMMARY %\
              (self.__spec.name, "Pass", self.__timer.TotalTime()))
        for v in self.__verifs:
            v.PrintResultSummary()

    def Main(self):
        Logger.info("Starting Testcase: %s" % self.__spec.name)
        self.__status = types.result.PASS
        self.__timer.Start()
        self.__execute()
        self.__timer.Stop()
        return types.status.SUCCESS
