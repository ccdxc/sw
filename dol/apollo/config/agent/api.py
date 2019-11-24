#! /usr/bin/python3
import os
import pdb
import grpc
import enum

import batch_pb2_grpc as batch_pb2_grpc
import device_pb2_grpc as device_pb2_grpc
import vpc_pb2_grpc as vpc_pb2_grpc
import subnet_pb2_grpc as subnet_pb2_grpc
import tunnel_pb2_grpc as tunnel_pb2_grpc
import vnic_pb2_grpc as vnic_pb2_grpc
import mapping_pb2_grpc as mapping_pb2_grpc
import route_pb2_grpc as route_pb2_grpc
import policy_pb2_grpc as policy_pb2_grpc
import mirror_pb2_grpc as mirror_pb2_grpc
import nh_pb2_grpc as nh_pb2_grpc
import service_pb2_grpc as service_pb2_grpc
import tags_pb2_grpc as tags_pb2_grpc
import meter_pb2_grpc as meter_pb2_grpc
import interface_pb2_grpc as interface_pb2_grpc

import device_pb2 as device_pb2
import interface_pb2 as interface_pb2
import mapping_pb2 as mapping_pb2
import meter_pb2 as meter_pb2
import mirror_pb2 as mirror_pb2
import nh_pb2 as nh_pb2
import policy_pb2 as policy_pb2
import route_pb2 as route_pb2
import subnet_pb2 as subnet_pb2
import tags_pb2 as tags_pb2
import tunnel_pb2 as tunnel_pb2
import vpc_pb2 as vpc_pb2
import vnic_pb2 as vnic_pb2

from infra.common.glopts  import GlobalOptions
from infra.common.logging import logger

MAX_BATCH_SIZE = 64

class ApiOps(enum.IntEnum):
    NONE = 0
    CREATE = 1
    DELETE = 2
    UPDATE = 3
    GET = 4
    START = 5
    COMMIT = 6
    ABORT = 7
    MAX = 8

class ObjectTypes(enum.IntEnum):
    NONE = 0
    BATCH = 1
    DEVICE = 2
    VPC = 3
    SUBNET = 4
    TUNNEL = 5
    VNIC = 6
    MAPPING = 7
    ROUTE = 8
    POLICY = 9
    MIRROR = 10
    NEXTHOP = 11
    NEXTHOPGROUP = 12
    SVCMAPPING = 13
    METER = 14
    TAG = 15
    INTERFACE = 16
    MAX = 17

class ClientModule:
    def __init__(self, module, msg_prefix):
        self.__module = module
        self.__msg_reqs = [None] * ApiOps.MAX
        self.__set_msg_reqs(msg_prefix)
        return

    def __set_one_msg_req(self, op, msgreq):
        self.__msg_reqs[op] = getattr(self.__module, msgreq, None)
        logger.info("Setting %s for OP: %d" % (self.__msg_reqs[op], op))
        return

    def __set_msg_reqs(self, p):
        self.__set_one_msg_req(ApiOps.CREATE, "%sRequest"%p)
        self.__set_one_msg_req(ApiOps.DELETE, "%sDeleteRequest"%p)
        self.__set_one_msg_req(ApiOps.UPDATE, "%sUpdateRequest"%p)
        self.__set_one_msg_req(ApiOps.GET, "%sGetRequest"%p)
        return

    def MsgReq(self, op):
        return self.__msg_reqs[op]

class ClientStub:
    def __init__(self, stubclass, channel, rpc_prefix):
        self.__stub = stubclass(channel)
        self.__rpcs = [None] * ApiOps.MAX
        self.__set_rpcs(rpc_prefix)
        return

    def __set_one_rpc(self, op, rpc):
        self.__rpcs[op] = getattr(self.__stub, rpc, None)
        if not self.__rpcs[op]:
            logger.warn("%s is None for OP: %d" % (rpc, op))
        return

    def __set_rpcs(self, p):
        self.__set_one_rpc(ApiOps.CREATE, "%sCreate"%p)
        self.__set_one_rpc(ApiOps.DELETE, "%sDelete"%p)
        self.__set_one_rpc(ApiOps.UPDATE, "%sUpdate"%p)
        self.__set_one_rpc(ApiOps.GET, "%sGet"%p)
        self.__set_one_rpc(ApiOps.START, "%sStart"%p)
        self.__set_one_rpc(ApiOps.COMMIT, "%sCommit"%p)
        self.__set_one_rpc(ApiOps.ABORT, "%sAbort"%p)
        return

    def Rpc(self, op, objs):
        resps = []
        for obj in objs:
            resps.append(self.__rpcs[op](obj))
        return resps

class ApolloAgentClientRequest:
    def __init__(self, stub, req,):
        self.__stub = stub
        return

class ApolloAgentClient:
    def __init__(self):
        self.__channel = None
        self.__stubs = [None] * ObjectTypes.MAX
        self.__msgreqs = [None] * ObjectTypes.MAX
        self.__create_msgreq_table()
        self.__connect()
        self.__create_stubs()
        return

    def __get_agent_port(self):
        try:
            port = os.environ['AGENT_GRPC_PORT']
        except:
            port = '9999'
        return port;

    def __get_agent_ip(self):
        try:
            agentip = os.environ['AGENT_GRPC_IP']
        except:
            agentip = 'localhost'
        return agentip


    def __connect(self):
        if GlobalOptions.dryrun: return
        endpoint = "%s:%s" % (self.__get_agent_ip(), self.__get_agent_port())
        logger.info("Connecting to Agent %s" % endpoint)
        self.__channel = grpc.insecure_channel(endpoint)
        logger.info("Waiting for Agent to be ready ...")
        grpc.channel_ready_future(self.__channel).result()
        logger.info("Connected to Agent!")
        return

    def __create_stubs(self):
        if GlobalOptions.dryrun: return
        self.__stubs[ObjectTypes.BATCH] = ClientStub(batch_pb2_grpc.BatchSvcStub,
                                                     self.__channel, 'Batch')
        self.__stubs[ObjectTypes.DEVICE] = ClientStub(device_pb2_grpc.DeviceSvcStub,
                                                      self.__channel, 'Device')
        self.__stubs[ObjectTypes.INTERFACE] = ClientStub(interface_pb2_grpc.IfSvcStub,
                                                      self.__channel, 'Interface')
        self.__stubs[ObjectTypes.VPC] = ClientStub(vpc_pb2_grpc.VPCSvcStub,
                                                   self.__channel, 'VPC')
        self.__stubs[ObjectTypes.SUBNET] = ClientStub(subnet_pb2_grpc.SubnetSvcStub,
                                                      self.__channel, 'Subnet')
        self.__stubs[ObjectTypes.TUNNEL] = ClientStub(tunnel_pb2_grpc.TunnelSvcStub,
                                                      self.__channel, 'Tunnel')
        self.__stubs[ObjectTypes.VNIC] = ClientStub(vnic_pb2_grpc.VnicSvcStub,
                                                      self.__channel, 'Vnic')
        self.__stubs[ObjectTypes.MAPPING] = ClientStub(mapping_pb2_grpc.MappingSvcStub,
                                                      self.__channel, 'Mapping')
        self.__stubs[ObjectTypes.ROUTE] = ClientStub(route_pb2_grpc.RouteSvcStub,
                                                      self.__channel, 'RouteTable')
        self.__stubs[ObjectTypes.POLICY] = ClientStub(policy_pb2_grpc.SecurityPolicySvcStub,
                                                      self.__channel, 'SecurityPolicy')
        self.__stubs[ObjectTypes.MIRROR] = ClientStub(mirror_pb2_grpc.MirrorSvcStub,
                                                      self.__channel, 'MirrorSession')
        self.__stubs[ObjectTypes.NEXTHOP] = ClientStub(nh_pb2_grpc.NhSvcStub,
                                                      self.__channel, 'Nexthop')
        self.__stubs[ObjectTypes.NEXTHOPGROUP] = ClientStub(nh_pb2_grpc.NhSvcStub,
                                                      self.__channel, 'NhGroup')
        self.__stubs[ObjectTypes.SVCMAPPING] = ClientStub(service_pb2_grpc.SvcStub,
                                                      self.__channel, 'SvcMapping')
        self.__stubs[ObjectTypes.TAG] = ClientStub(tags_pb2_grpc.TagSvcStub,
                                                      self.__channel, 'Tag')
        self.__stubs[ObjectTypes.METER] = ClientStub(meter_pb2_grpc.MeterSvcStub,
                                                      self.__channel, 'Meter')
        return

    def __create_msgreq_table(self):
        self.__msgreqs[ObjectTypes.DEVICE] = ClientModule(device_pb2, 'Device')
        self.__msgreqs[ObjectTypes.INTERFACE] = ClientModule(interface_pb2, 'Interface')
        self.__msgreqs[ObjectTypes.MAPPING] = ClientModule(mapping_pb2, 'Mapping')
        self.__msgreqs[ObjectTypes.METER] = ClientModule(meter_pb2, 'Meter')
        self.__msgreqs[ObjectTypes.MIRROR] = ClientModule(mirror_pb2, 'MirrorSession')
        self.__msgreqs[ObjectTypes.NEXTHOP] = ClientModule(nh_pb2, 'Nexthop')
        self.__msgreqs[ObjectTypes.NEXTHOPGROUP] = ClientModule(nh_pb2, 'NhGroup')
        self.__msgreqs[ObjectTypes.ROUTE] = ClientModule(route_pb2, 'RouteTable')
        self.__msgreqs[ObjectTypes.POLICY] = ClientModule(policy_pb2, 'SecurityPolicy')
        self.__msgreqs[ObjectTypes.SUBNET] = ClientModule(subnet_pb2, 'Subnet')
        self.__msgreqs[ObjectTypes.TAG] = ClientModule(tags_pb2, 'Tag')
        self.__msgreqs[ObjectTypes.TUNNEL] = ClientModule(tunnel_pb2, 'Tunnel')
        self.__msgreqs[ObjectTypes.VPC] = ClientModule(vpc_pb2, 'VPC')
        self.__msgreqs[ObjectTypes.VNIC] = ClientModule(vnic_pb2, 'Vnic')
        return

    def GetGRPCMsgReq(self, objtype, op):
        return self.__msgreqs[objtype].MsgReq(op)

    def Create(self, objtype, objs):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.CREATE, objs)

    def Delete(self, objtype, objs):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.DELETE, objs)

    def Update(self, objtype, objs):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.UPDATE, objs)

    def Get(self, objtype, objs):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.GET, objs)

    def Start(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.START, [ obj ])

    def Abort(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.ABORT, [ obj ])

    def Commit(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.COMMIT, [ obj ])

client = None
def Init():
    global client
    client = ApolloAgentClient()
