#! /usr/bin/python3
import pdb
from iota.harness.infra.utils.logger import Logger as Logger

import iota.harness.infra.store as store
import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.testcase as testcase
import iota.harness.infra.topology as topology

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.types_pb2 as types_pb2

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

class TestSuite:
    def __init__(self, spec):
        self.__spec = spec
        self.__tcs = []

        if GlobalOptions.testsuite and GlobalOptions.testsuite != self.Name():
            Logger.info("Skipping Testsuite: %s because of command-line filters." % self.Name())
            self.__valid = False
            return

        if GlobalOptions.mode != self.Mode():
            Logger.info("Skipping Testsuite: %s because of mode: %s" % (self.Name(), self.Mode()))
            self.__valid = False
            return

        Logger.info("Starting Testsuite: %s" % self.Name())
        self.__resolve_testcases()
        self.__resolve_teardown()
        self.__parse_setup()
        self.__valid = True
        return

    def GetImages(self):
        return self.__spec.images

    def GetTopology(self):
        return self.__topology

    def __resolve_testcases(self):
        for tc_spec in self.__spec.testcases:
            tc_spec.packages = self.__spec.packages
            if tc_spec.verifs:
                tc_spec.verifs.extend(self.__spec.common.verifs)
            tc = testcase.Testcase(tc_spec)
            self.__tcs.append(tc)
        return types.status.SUCCESS

    def __resolve_teardown(self):
        teardown_spec = getattr(self.__spec, 'teardown', [])
        if teardown_spec is None:
            return types.status.SUCCESS
        for s in self.__spec.teardown:
            Logger.debug("Resolving teardown module: %s" % s.step)
            s.step = loader.Import(s.step, self.__spec.packages)
        return types.status.SUCCESS

    def __parse_setup_topology(self):
        topospec = getattr(self.__spec.setup, 'topology', None)
        if not topospec:
            Logger.error("Error: No topology specified in the testsuite.")
            assert(0)
        self.__topology = topology.Topology(topospec)
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

    def __execute_testcases(self):
        for tc in self.__tcs:
            ret = tc.Main()
            if ret != types.status.SUCCESS and GlobalOptions.no_keep_going:
                return ret
        return types.status.SUCCESS

    def __print_summary(self):
        print("\nTestSuite Results: %s" % self.__spec.meta.name)
        print(types.HEADER_SUMMARY)
        print(types.FORMAT_TESTCASE_SUMMARY %\
              ("Testcase", "Result", "Duration"))
        print(types.HEADER_SUMMARY)
        for tc in self.__tcs:
            tc.PrintResultSummary()
        return types.status.SUCCESS

    def Name(self):
        return self.__spec.meta.name
    
    def Mode(self):
        return self.__spec.meta.mode

    def Main(self):
        if not self.__valid:
           return types.status.SUCCESS

        # Initialize Testbed for this testsuite
        status = store.GetTestbed().InitForTestsuite(self)
        if status != types.status.SUCCESS:
            return status

        status = self.__setup()
        if status != types.status.SUCCESS:
            return status

        self.result = self.__execute_testcases()

        self.__print_summary()

        return types.status.SUCCESS
