#! /usr/bin/python3
import pdb

import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.api as api
import iota.harness.infra.store as store

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.types_pb2 as types_pb2

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from iota.harness.infra.utils.logger import Logger as Logger

class _Testbed:
    def __init__(self):
        self.curr_ts = None     # Current Testsuite
        self.prev_ts = None     # Previous Testsute
        self.__node_ips = []
        
        self.__ipidx = 0
        self.__read_warmd_json()
        return

    def GetCurrentTestsuite(self):
        return self.curr_ts

    def __read_warmd_json(self):
        self.tbspec = parser.JsonParse(GlobalOptions.testbed_json)
        for k,v in self.tbspec.Instances.__dict__.items():
            self.__node_ips.append(v)
        return
    
    def __prepare_TestBedMsg(self, ts):
        msg = topo_pb2.TestBedMsg()
        if not ts:
            return msg
        if not GlobalOptions.rerun:
            msg.naples_image = ts.GetImages().naples
            msg.venice_image = ts.GetImages().venice
            msg.driver_sources = ts.GetImages().drivers

        # TBD: Get it from warmd.json
        msg.username = self.tbspec.Provision.Username
        msg.password = self.tbspec.Provision.Password
        msg.testbed_id = int(self.tbspec.DataNetworks.DataSwitch.Port)

        for node_ip in self.__node_ips:
            node_msg = msg.nodes.add()
            node_msg.type = topo_pb2.TESTBED_NODE_TYPE_SIM
            node_msg.ip_address = node_ip
        return msg

    def __cleanup_testbed(self):
        msg = self.__prepare_TestBedMsg(self.prev_ts)
        resp = api.CleanupTestbed(msg)
        if resp is None:
            Logger.error("Failed to cleanup testbed: ")
            return types.status.FAILURE
        self.data_vlans = None
        return types.status.SUCCESS

    def __init_testbed(self):
        msg = self.__prepare_TestBedMsg(self.curr_ts)
        resp = api.InitTestbed(msg)
        if resp is None:
            Logger.error("Failed to initialize testbed: ")
            return types.status.FAILURE
        self.data_vlans = resp.allocated_vlans
        return types.status.SUCCESS

    def InitForTestsuite(self, ts):
        self.prev_ts = self.curr_ts
        self.curr_ts = ts
        #status = self.__cleanup_testbed()
        #if status != types.status.SUCCESS:
        #    return status

        status = self.__init_testbed()
        return status

    def ReserveNodeIpAddress(self):
        if len(self.__node_ips) and self.__ipidx < len(self.__node_ips):
            node_ip = self.__node_ips[self.__ipidx]
            self.__ipidx += 1
        else:
            Logger.error("No Nodes available in Testbed.")
            assert(0)
        return node_ip

    def GetDataVlans(self):
        resp = []
        for v in self.data_vlans:
            resp.append(v)
        return resp

__testbed = _Testbed()
store.SetTestbed(__testbed)
