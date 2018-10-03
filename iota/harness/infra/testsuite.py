#! /usr/bin/python3
import pdb
from iota.harness.infra.testbed import Testbed as Testbed
from iota.harness.infra.utils.logger import Logger as Logger

import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.testcase as testcase

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.types_pb2 as types_pb2

class TestSuite:
    def __init__(self, spec):
        self.__spec = spec
        self.__tcs = []
        
        self.__resolve_testcases()
        self.__resolve_teardown()
        self.__parse_setup()
        return

    def GetImages(self):
        return self.__spec.images

    def __resolve_testcases(self):
        for tc_spec in self.__spec.testcases:
            tc_spec.packages = self.__spec.packages
            tc_spec.verifs.extend(self.__spec.common.verifs)
            tc = testcase.Testcase(tc_spec)
            self.__tcs.append(tc)
        return

    def __resolve_teardown(self):
        for s in self.__spec.teardown:
            Logger.debug("Resolving teardown module: %s" % s.step)
            s.step = loader.Import(s.step, self.__spec.packages)
        return

    def __parse_setup_topology(self):
        topospec = getattr(self.__spec.setup, 'topology', None)
        if not topospec:
            Logger.error("Error: No topology specified in the testsuite.")
            assert(0)
        self.__topology = parser.YmlParse(self.__spec.setup.topology)
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

    def __setup_topology(self):
        Logger.info("Adding Nodes:")
        req = topo_pb2.NodeMsg()
        req.node_op = topo_pb2.ADD
        
        for n in self.__topology.nodes:
            node_req = req.nodes.add()
            node_req.type = topo_pb2.PersonalityType.Value(n.role)
            n.ip_address = Testbed.ReserveNodeIpAddress()
            node_req.ip_address = n.ip_address
            node_req.node_name = n.name
            Logger.info("- %s: %s (%s)" % (n.name, n.ip_address, n.role))

        resp = api.AddNodes(req)
        if resp.api_response.api_status != types_pb2.API_STATUS_OK:
            Logger.error("Failed to add Nodes: ",
                         types_pb2.APIResponseType.Name(resp.api_response.api_status))
            for n in resp.nodes:
                Logger.error(" - %s: " % types_pb2.APIResponseType.Name(n.node_status))
            return types.status.FAILURE

        return types.status.SUCCESS

    def __setup_config(self):
        return types.status.SUCCESS

    def __setup(self):
        ret = self.__setup_topology()
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

    def Main(self):
        Logger.info("Running Testsuite: %s" % self.__spec.meta.name)
        
        # Initialize Testbed for this testsuite
        Testbed.InitForTestsuite(self)

        self.__setup()

        self.result = self.__execute_testcases()

        self.__print_summary()

        return types.status.SUCCESS
