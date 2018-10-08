#! /usr/bin/python3
import pdb

import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.api as api

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

    def __read_warmd_json(self):
        self.tbspec = parser.JsonParse(GlobalOptions.testbed_json)
        for k,v in self.tbspec.Instances.__dict__.items():
            self.__node_ips.append(v)
        return
    
    def __prepare_TestBedMsg(self, ts):
        msg = topo_pb2.TestBedMsg()
        if not ts:
            return msg
        if getattr(self.tbspec, 'n3k', None):
            msg.switch_port_id = int(self.tbspec.n3k.port)
        msg.naples_image = ts.GetImages().naples
        msg.venice_image = ts.GetImages().venice
        msg.driver_sources = ts.GetImages().drivers

        # TBD: Get it from warmd.json
        msg.user = "vm"
        msg.passwd = "vm"

        for node_ip in self.__node_ips:
            msg.ip_address.append(node_ip)
        return msg

    def __cleanup_testbed(self):
        msg = self.__prepare_TestBedMsg(self.prev_ts)
        resp = api.CleanupTestbed(msg)
        if resp.api_response.api_status != types_pb2.API_STATUS_OK:
            Logger.error("Failed to initialize testbed: ",
                         types_pb2.APIResponseType.Name(resp.api_response.api_status))
            Logger.error("               error message: ", resp.api_response.error_msg)
            return types.status.FAILURE
        self.data_vlans = None
        return types.status.SUCCESS

    def __init_testbed(self):
        msg = self.__prepare_TestBedMsg(self.curr_ts)
        resp = api.InitTestbed(msg)
        if resp.api_response.api_status != types_pb2.API_STATUS_OK:
            Logger.error("Failed to initialize testbed: ",
                         types_pb2.APIResponseType.Name(resp.api_response.api_status))
            Logger.error("               error message: ", resp.api_response.error_msg)
            return types.status.FAILURE
        self.data_vlans = resp.allocated_vlans
        return types.status.SUCCESS

    def InitForTestsuite(self, ts):
        self.prev_ts = self.curr_ts
        self.curr_ts = ts
        status = self.__cleanup_testbed()
        if status != types.status.SUCCESS:
            return status

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

Testbed = _Testbed()
