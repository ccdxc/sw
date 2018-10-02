#! /usr/bin/python3
import pdb
from iota.harness.infra.testbed import Testbed as Testbed
from iota.harness.infra.utils.logger import Logger as Logger

import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader
import iota.harness.infra.svc as svc

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.types_pb2 as types_pb2

class TestSuite:
    def __init__(self, spec):
        self.__spec = spec
        self.__resolve_common_verifs()
        self.__resolve_testcases()
        self.__resolve_teardown()
        self.__parse_topology()
        return

    def GetImages(self):
        return self.__spec.images

    def __resolve_common_verifs(self):
        for cv in self.__spec.common.verifs:
            Logger.debug("Resolving common verif module: %s" % cv.step)
            cv.step = loader.Import(cv.step, self.__spec.packages)
        return

    def __resolve_testcases(self):
        for tc in self.__spec.testcases:
            Logger.debug("Resolving testcase module: %s" % tc.testcase)
            tc.testcase = loader.Import(tc.testcase, self.__spec.packages)
            tc.verifs = getattr(tc, 'verifs', [])
            for v in tc.verifs:
                Logger.debug("Resolving testcase verif module: %s" % v.step)
                v.step = loader.Import(v.step, self.__spec.packages)
            tc.verifs.extend(self.__spec.common.verifs)
        return

    def __resolve_teardown(self):
        for s in self.__spec.teardown:
            Logger.debug("Resolving teardown module: %s" % s.step)
            s.step = loader.Import(s.step, self.__spec.packages)
        return

    def __parse_topology(self):
        self.__topology = parser.YmlParse(self.__spec.topology)
        return

    def __init_nodes(self):
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

        resp = svc.AddNodes(req)
        if resp.api_response.api_status != types_pb2.API_STATUS_OK:
            Logger.error("Failed to add Nodes: ",
                         types_pb2.APIResponseType.Name(resp.api_response.api_status))
            for n in resp.nodes:
                Logger.error(" - %s: " % types_pb2.APIResponseType.Name(n.node_status))
            return types.status.FAILURE

        return types.status.SUCCESS

    def Main(self):
        Logger.info("Running Testsuite: %s" % self.__spec.meta.suite)
        # Initialize Testbed for this testsuite
        Testbed.InitForTestsuite(self)

        self.__init_nodes()
        return
