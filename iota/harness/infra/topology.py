#! /usr/bin/python3
import os

from iota.harness.infra.utils.logger import Logger as Logger

import iota.harness.infra.store as store
import iota.harness.infra.resmgr as resmgr
import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.testcase as testcase

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.types_pb2 as types_pb2


class Node(object):
    def __init__(self, spec):
        self.__spec = spec
        self.__name = spec.name
        self.__ip_address = store.GetTestbed().ReserveNodeIpAddress()
        self.__role = topo_pb2.PersonalityType.Value(spec.role)

        self.__control_ip = resmgr.ControlIpAllocator.Alloc()
        self.__control_intf = "eth1"

        self.__data_intfs = [ "eth2", "eth3" ]
        Logger.info("- New Node: %s: %s (%s)" % (spec.name, self.__ip_address, spec.role))
        return

    def Name(self):
        return self.__name

    def Role(self):
        return self.__role

    def ControlIpAddress(self):
        return self.__control_ip

    def MgmtIpAddress(self):
        return self.__ip_address

    def AddToNodeMsg(self, msg, topology, testsuite):
        msg.type = self.__role
        msg.image = ""
        msg.ip_address = self.__ip_address
        msg.name = self.__name

        if self.Role() == topo_pb2.PERSONALITY_VENICE:
            msg.venice_config.control_intf = self.__control_intf
            msg.venice_config.control_ip = str(self.__control_ip)
            msg.image = os.path.basename(testsuite.GetImages().venice)
            for n in topology.Nodes():
                if n.Role() != topo_pb2.PERSONALITY_VENICE: continue
                peer_msg = msg.venice_config.venice_peers.add()
                peer_msg.host_name = n.Name()
                peer_msg.ip_address = str(n.ControlIpAddress())
        else:
            msg.naples_config.control_intf = self.__control_intf
            msg.naples_config.control_ip = str(self.__control_ip)
            msg.image = os.path.basename(testsuite.GetImages().naples)
            for data_intf in self.__data_intfs:
                msg.naples_config.data_intfs.append(data_intf)
            #for n in topology.Nodes():
            #    if n.Role() != topo_pb2.PERSONALITY_VENICE: continue
            #    msg.naples_config.venice_ips.append(str(n.ControlIpAddress()))

        return types.status.SUCCESS

class Topology(object):
    def __init__(self, spec):
        self.__nodes = []

        assert(spec)
        self.__spec = parser.YmlParse(spec)
        self.__parse_nodes()
        return

    def __parse_nodes(self):
        for node_spec in self.__spec.nodes:
            node = Node(node_spec)
            self.__nodes.append(node)
        return

    def Nodes(self):
        return self.__nodes

    def Setup(self, testsuite):
        Logger.info("Adding Nodes:")
        req = topo_pb2.NodeMsg()
        req.node_op = topo_pb2.ADD
        
        for node in self.__nodes:
            msg = req.nodes.add()
            ret = node.AddToNodeMsg(msg, self, testsuite)
            assert(ret == types.status.SUCCESS)

        resp = api.AddNodes(req)
        if resp.api_response.api_status != types_pb2.API_STATUS_OK:
            Logger.error("Failed to add Nodes: ",
                         types_pb2.APIResponseType.Name(resp.api_response.api_status))
            Logger.error("      error message: ", resp.api_response.error_msg)
            for n in resp.nodes:
                Logger.error(" - %s: " % types_pb2.APIResponseType.Name(n.node_status))
            return types.status.FAILURE

        return types.status.SUCCESS

    def GetVeniceMgmtIpAddresses(self):
        ips = []
        for n in self.__nodes:
            if n.Role() == topo_pb2.PERSONALITY_VENICE:
                ips.append(n.MgmtIpAddress())
        return ips

    def GetVeniceHostnames(self):
        ips = []
        for n in self.__nodes:
            if n.Role() == topo_pb2.PERSONALITY_VENICE:
                ips.append(n.Name())
        return ips
