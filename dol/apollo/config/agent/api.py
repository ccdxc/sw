#! /usr/bin/python3
import inspect
import os
import grpc
import enum
import requests
import time
import urllib3
import socket

# operd proto
import oper_pb2_grpc as oper_pb2_grpc
import alerts_pb2_grpc as alerts_pb2_grpc

# pds proto
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
import dhcp_pb2_grpc as dhcp_pb2_grpc
import nat_pb2_grpc as nat_pb2_grpc
import bgp_pb2_grpc as bgp_pb2_grpc
import cp_route_pb2_grpc as cp_route_pb2_grpc
import evpn_pb2_grpc as evpn_pb2_grpc
import policer_pb2_grpc as policer_pb2_grpc

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
import dhcp_pb2 as dhcp_pb2
import nat_pb2 as nat_pb2
import bgp_pb2 as bgp_pb2
import cp_route_pb2 as cp_route_pb2
import evpn_pb2 as evpn_pb2
import policer_pb2 as policer_pb2
import types_pb2 as types_pb2
from types_pb2 import ServiceRequestOp as ServiceOp

from infra.common.glopts  import GlobalOptions
from infra.common.logging import logger

# Connection timeout - 3mins
MAX_CONNECT_TIMEOUT = 180
MAX_MSG_LEN = 1024 * 1024 * 100
# RPC Timeout - 20mins
MAX_GRPC_WAIT = 1200
MAX_BATCH_SIZE = 64
SVC_SERVER_SOCKET_PATH = "/var/run/pds_svc_server_sock"
SOCK_BUF_LEN = 1024 * 1024

class AgentPorts(enum.IntEnum):
    NONE = 0
    DSCAGENTREST = 8888
    PDSAGENT = 11357
    OPERD = 11359
    PEN_OPER = 11360

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
    LMAPPING = 7
    ROUTE = 8
    POLICY = 9
    MIRROR = 10
    NEXTHOP = 11
    NEXTHOPGROUP = 12
    SVCMAPPING = 13
    METER = 14
    TAG = 15
    INTERFACE = 16
    DHCP_RELAY = 17
    NAT = 18
    RMAPPING = 19
    POLICER = 20
    BGP = 21
    BGP_PEER = 22
    BGP_PEER_AF = 23
    BGP_NLRI_PREFIX = 24
    BGP_EVPN_EVI = 25
    BGP_EVPN_EVI_RT = 26
    BGP_EVPN_IP_VRF = 27
    BGP_EVPN_IP_VRF_RT = 28
    STATIC_ROUTE = 29
    SECURITY_PROFILE = 30
    DHCP_PROXY = 31
    MAX = 32

class OperdObjectTypes(enum.IntEnum):
    NONE = 0
    TECHSUPPORT = 1
    MAX = 2

class PenOperObjectTypes(enum.IntEnum):
    NONE = 0
    ALERTS = 1
    METRICS = 2
    MAX = 3

class Transport(enum.IntEnum):
    GRPC = 0
    UDS = 1
    MAX = 2

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
        self.__set_one_msg_req(ApiOps.UPDATE, "%sRequest"%p)
        # TODO this will change if metaswitch protos change
        if "BGP" in p or "Evpn" in p or "CPStatic" in p:
            self.__set_one_msg_req(ApiOps.GET, "%sRequest"%p)
        else:
            self.__set_one_msg_req(ApiOps.GET, "%sGetRequest"%p)
        return

    def MsgReq(self, op):
        return self.__msg_reqs[op]

class ClientResponseModule:
    def __init__(self, module, msg_prefix):
        self.__module = module
        self.__msg_resps = [None] * ApiOps.MAX
        self.__set_msg_resps(msg_prefix)
        return

    def __set_one_msg_resp(self, op, msgresp):
        self.__msg_resps[op] = getattr(self.__module, msgresp, None)
        logger.info("Setting %s for OP: %d" % (self.__msg_resps[op], op))
        return

    def __set_msg_resps(self, p):
        self.__set_one_msg_resp(ApiOps.CREATE, "%sResponse"%p)
        self.__set_one_msg_resp(ApiOps.DELETE, "%sDeleteResponse"%p)
        self.__set_one_msg_resp(ApiOps.UPDATE, "%sResponse"%p)
        self.__set_one_msg_resp(ApiOps.GET, "%sGetResponse"%p)
        return

    def MsgResp(self, op):
        return self.__msg_resps[op]

class ClientRESTModule:
    def __init__(self, ip, uri):
        self.url = f"https://{ip}:{AgentPorts.DSCAGENTREST.value}{uri}"
        return

    def Create(self, objs):
        resps = []
        for obj in objs:
            pdata = obj.PopulateAgentJson()
            logger.info("Obj:%s Posting to URL %s"%(obj.GID(), self.url))
            if not pdata:
                logger.info("Invalid Post Data from object %s"%obj.GID())
                continue
            logger.info("PostData: %s"%pdata)
            if not GlobalOptions.dryrun:
                rdata = requests.post(self.url, pdata, verify=False)
                if rdata.status_code != 200:
                    logger.error("Obj:%s POST FAILED [%d] to URL %s"%(obj.GID(), rdata.status_code, self.url))
                else:
                    resps.append(rdata)
            else:
                resps.append(True)
        return resps

    def Update(self, objs):
        resps = []
        for obj in objs:
            url = "%s%s"%(self.url, obj.GetRESTPath())
            pdata = obj.PopulateAgentJson()
            logger.info("Obj:%s Put to URL %s"%(obj.GID(), url))
            if not pdata:
                logger.info("Invalid Put Data from object %s"%obj.GID())
                continue
            logger.info("PutData: %s"%pdata)
            if not GlobalOptions.dryrun:
                rdata = requests.put(url, pdata, verify=False)
                if rdata.status_code != 200:
                    logger.error("Obj:%s PUT FAILED [%d] to URL %s"%(obj.GID(), rdata.status_code, url))
                else:
                    resps.append(rdata)
            else:
                resps.append(True)
        return resps

    def Get(self):
        rdata = requests.get(self.url, verify=False)
        if rdata.status_code != 200:
            logger.error("GET FAILED [%d] to URL %s"%(rdata.status_code, self.url))
            return
        return rdata.json()

    def Delete(self, objs):
        resps = []
        for obj in objs:
            url = "%s%s"%(self.url, obj.GetRESTPath())
            logger.info("Obj:%s Delete URL %s"%(obj.GID(), url))
            if not GlobalOptions.dryrun:
                rdata = requests.delete(url, verify=False)
                if rdata.status_code != 200:
                    logger.error("Obj:%s DELETE FAILED [%d] URL %s"%(obj.GID(), rdata.status_code, url))
                else:
                    resps.append(rdata)
            else:
                resps.append(True)
        return resps

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
            resps.append(self.__rpcs[op](obj, timeout=MAX_GRPC_WAIT))
        return resps

    def DoRpc(self, rpcname, objs):
        resps = []
        rpc = getattr(self.__stub, rpcname, None)
        if not rpc:
            logger.error(f"Invalid RPC {rpcname}")
            return resps
        for obj in objs:
            resps.append(rpc(obj, timeout=MAX_GRPC_WAIT))
        return resps

class RetryOnFailureInterceptors(grpc.UnaryUnaryClientInterceptor,
                                 grpc.StreamUnaryClientInterceptor):
    def __init__(self):
        self.__backoff_timeout = 5
        self.__max_retries = 5
        self.__status_codes2retry = [grpc.StatusCode.UNAVAILABLE]

    def __continue_rpc(self, continuation, call_info, req):
        retry = False
        res = continuation(call_info, req)
        if not isinstance(res, grpc.RpcError):
            # return if this is not error case
            return res, retry
        # get the error code
        rc = res.code()
        if rc not in self.__status_codes2retry:
            # return if there is no need to retry for this error code
            return res, retry
        # need to retry for this error
        return res, True

    def __intercept_rpc(self, continuation, call_info, req):
        for __retry in range(self.__max_retries):
            res, retry = self.__continue_rpc(continuation, call_info, req)
            if not retry:
                # return in case of success / uninterested responses
                return res
            logger.info(f"sleeping for {self.__backoff_timeout}s before rpc retry")
            time.sleep(self.__backoff_timeout)
        logger.error("Max rpc retries elapsed - agent likely crashed")
        return res

    def intercept_unary_unary(self, continuation, client_call_details, request):
        # Intercepts a unary-unary invocation asynchronously
        return self.__intercept_rpc(continuation, client_call_details, request)

    def intercept_stream_unary(self, continuation, client_call_details, request_iterator):
        # Intercepts a stream-unary invocation asynchronously
        return self.__intercept_rpc(continuation, client_call_details, request_iterator)

class AgentClientBase:
    rpcInterceptors = (RetryOnFailureInterceptors(),)

    def __init__(self, name, objTypes, ip = None):
        self.Name = name
        self.Channel = None
        self.Stubs = [None] * objTypes.MAX
        self.MsgReqs = [None] * objTypes.MAX
        self.MsgResps = [None] * ObjectTypes.MAX
        self.IPAddr = self.GetAgentIP(ip)
        self.Port = self.GetAgentPort()
        self.Endpoint = f"{self.IPAddr}:{self.Port}"
        self.Transport = Transport.GRPC
        self.CreateMsgReqTable()
        self.CreateChannel()
        if not self.Connect():
            assert (0), f"Failed to connect to {self.Name} at {self.Endpoint}"
        self.CreateStubs()
        return

    def __unimplemented(self):
        UnimplementedError = f"'{inspect.stack()[1].function}' method is NOT "\
                             f"implemented by class: {self.__class__}"
        logger.error(UnimplementedError)
        assert (0), UnimplementedError

    def GetAgentPort(self):
        self.__unimplemented()

    def GetAgentIP(self, agentip):
        if agentip: return agentip
        try:
            agentip = os.environ['AGENT_GRPC_IP']
        except:
            agentip = 'localhost'
        return agentip

    def CreateChannel(self):
        logger.info(f"Creating grpc channel to {self.Name} at {self.Endpoint}")
        channel = grpc.insecure_channel(
                self.Endpoint,
                options=[
                        ('grpc.max_receive_message_length', MAX_MSG_LEN),
                        ('grpc.max_send_message_length', MAX_MSG_LEN),
                ]
        )
        self.Channel = grpc.intercept_channel(channel, *AgentClientBase.rpcInterceptors)

    def Connect(self):
        if GlobalOptions.dryrun: return True
        logger.info(f"Waiting for {self.Name} at {self.Endpoint} to be ready ...")
        try:
            grpc.channel_ready_future(self.Channel).result(MAX_CONNECT_TIMEOUT)
        except:
            logger.error(f"Error establishing grpc connection to {self.Name} at {self.Endpoint}")
            return False
        logger.info(f"Established grpc connection to {self.Name} at {self.Endpoint}")
        return True

    def CreateStubs(self):
        self.__unimplemented()

    def CreateMsgReqTable(self):
        self.__unimplemented()

    def GetGRPCMsgReq(self, objtype, op):
        return self.MsgReqs[objtype].MsgReq(op)

    def UdsSend(self, oper, objtype, objs):
        self.__unimplemented()

    def Create(self, objtype, objs):
        if GlobalOptions.dryrun: return True
        if self.Transport == Transport.UDS:
            return self.UdsSend(ApiOps.CREATE, objtype, objs)
        return self.Stubs[objtype].Rpc(ApiOps.CREATE, objs)

    def Delete(self, objtype, objs):
        if GlobalOptions.dryrun: return True
        if self.Transport == Transport.UDS:
            return self.UdsSend(ApiOps.DELETE, objtype, objs)
        return self.Stubs[objtype].Rpc(ApiOps.DELETE, objs)

    def Update(self, objtype, objs):
        if GlobalOptions.dryrun: return True
        if self.Transport == Transport.UDS:
            return self.UdsSend(ApiOps.UPDATE, objtype, objs)
        return self.Stubs[objtype].Rpc(ApiOps.UPDATE, objs)

    def Get(self, objtype, objs):
        if GlobalOptions.dryrun: return
        if self.Transport == Transport.UDS:
            return self.UdsSend(ApiOps.GET, objtype, objs)
        return self.Stubs[objtype].Rpc(ApiOps.GET, objs)

    def Request(self, objtype, rpcname, objs):
        if GlobalOptions.dryrun: return
        return self.Stubs[objtype].DoRpc(rpcname, objs)

class OperdClient(AgentClientBase):
    def __init__(self, ip = None):
        super().__init__('operd', OperdObjectTypes, ip)
        return

    def GetAgentPort(self):
        try:
            port = os.environ['OPERD_GRPC_PORT']
        except:
            port = f'{AgentPorts.OPERD.value}'
        return port;

    def CreateStubs(self):
        if GlobalOptions.dryrun: return
        self.Stubs[OperdObjectTypes.TECHSUPPORT] = ClientStub(oper_pb2_grpc.OperSvcStub,
                                                              self.Channel, 'Oper')
        return

    def CreateMsgReqTable(self):
        return

class PenOperClient(AgentClientBase):
    def __init__(self, ip = None):
        super().__init__('pen_oper', PenOperObjectTypes, ip)
        return

    def GetAgentPort(self):
        try:
            port = os.environ['PEN_OPER_GRPC_PORT']
        except:
            port = f'{AgentPorts.PEN_OPER.value}'
        return port;

    def CreateStubs(self):
        if GlobalOptions.dryrun: return
        self.Stubs[PenOperObjectTypes.ALERTS] = ClientStub(alerts_pb2_grpc.AlertsSvcStub,
                                                           self.Channel, 'Alerts')
        return

    def CreateMsgReqTable(self):
        return

class PdsAgentClient(AgentClientBase):
    def __init__(self, ip = None):
        super().__init__('agent', ObjectTypes, ip)
        # TODO: split netagent client
        if GlobalOptions.netagent:
            self.RestReqs = [None] * ObjectTypes.MAX
            self.__create_restreq_table()
            urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

        if (os.environ.get("USE_UDS", None)):
            self.Transport = Transport.UDS
            self.CreateMsgRespTable()
        return

    def GetAgentPort(self):
        try:
            port = os.environ['AGENT_GRPC_PORT']
        except:
            port = f'{AgentPorts.PDSAGENT.value}'
        return port;

    def CreateStubs(self):
        if GlobalOptions.dryrun: return
        self.Stubs[ObjectTypes.BATCH] = ClientStub(batch_pb2_grpc.BatchSvcStub,
                                                     self.Channel, 'Batch')
        self.Stubs[ObjectTypes.DEVICE] = ClientStub(device_pb2_grpc.DeviceSvcStub,
                                                      self.Channel, 'Device')
        self.Stubs[ObjectTypes.INTERFACE] = ClientStub(interface_pb2_grpc.IfSvcStub,
                                                      self.Channel, 'Interface')
        self.Stubs[ObjectTypes.VPC] = ClientStub(vpc_pb2_grpc.VPCSvcStub,
                                                   self.Channel, 'VPC')
        self.Stubs[ObjectTypes.SUBNET] = ClientStub(subnet_pb2_grpc.SubnetSvcStub,
                                                      self.Channel, 'Subnet')
        self.Stubs[ObjectTypes.TUNNEL] = ClientStub(tunnel_pb2_grpc.TunnelSvcStub,
                                                      self.Channel, 'Tunnel')
        self.Stubs[ObjectTypes.VNIC] = ClientStub(vnic_pb2_grpc.VnicSvcStub,
                                                      self.Channel, 'Vnic')
        self.Stubs[ObjectTypes.LMAPPING] = ClientStub(mapping_pb2_grpc.MappingSvcStub,
                                                      self.Channel, 'Mapping')
        self.Stubs[ObjectTypes.RMAPPING] = ClientStub(mapping_pb2_grpc.MappingSvcStub,
                                                      self.Channel, 'Mapping')
        self.Stubs[ObjectTypes.ROUTE] = ClientStub(route_pb2_grpc.RouteSvcStub,
                                                      self.Channel, 'RouteTable')
        self.Stubs[ObjectTypes.POLICY] = ClientStub(policy_pb2_grpc.SecurityPolicySvcStub,
                                                      self.Channel, 'SecurityPolicy')
        self.Stubs[ObjectTypes.SECURITY_PROFILE] = ClientStub(policy_pb2_grpc.SecurityPolicySvcStub,
                                                      self.Channel, 'SecurityProfile')
        self.Stubs[ObjectTypes.MIRROR] = ClientStub(mirror_pb2_grpc.MirrorSvcStub,
                                                      self.Channel, 'MirrorSession')
        self.Stubs[ObjectTypes.NEXTHOP] = ClientStub(nh_pb2_grpc.NhSvcStub,
                                                      self.Channel, 'Nexthop')
        self.Stubs[ObjectTypes.NEXTHOPGROUP] = ClientStub(nh_pb2_grpc.NhSvcStub,
                                                      self.Channel, 'NhGroup')
        self.Stubs[ObjectTypes.SVCMAPPING] = ClientStub(service_pb2_grpc.SvcStub,
                                                      self.Channel, 'SvcMapping')
        self.Stubs[ObjectTypes.TAG] = ClientStub(tags_pb2_grpc.TagSvcStub,
                                                      self.Channel, 'Tag')
        self.Stubs[ObjectTypes.METER] = ClientStub(meter_pb2_grpc.MeterSvcStub,
                                                      self.Channel, 'Meter')
        self.Stubs[ObjectTypes.DHCP_RELAY] = ClientStub(dhcp_pb2_grpc.DHCPSvcStub,
                                                      self.Channel, 'DHCPPolicy')
        self.Stubs[ObjectTypes.DHCP_PROXY] = ClientStub(dhcp_pb2_grpc.DHCPSvcStub,
                                                      self.Channel, 'DHCPPolicy')
        self.Stubs[ObjectTypes.NAT] = ClientStub(nat_pb2_grpc.NatSvcStub,
                                                      self.Channel, 'NatPortBlock')
        self.Stubs[ObjectTypes.POLICER] = ClientStub(policer_pb2_grpc.PolicerSvcStub,
                                                      self.Channel, 'Policer')
        self.Stubs[ObjectTypes.BGP] = ClientStub(bgp_pb2_grpc.BGPSvcStub,
                                                   self.Channel, 'BGP')
        self.Stubs[ObjectTypes.BGP_PEER] = ClientStub(bgp_pb2_grpc.BGPSvcStub,
                                                        self.Channel, 'BGPPeer')
        self.Stubs[ObjectTypes.BGP_PEER_AF] = ClientStub(bgp_pb2_grpc.BGPSvcStub,
                                                           self.Channel, 'BGPPeerAf')
        self.Stubs[ObjectTypes.BGP_EVPN_EVI] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.Channel, 'EvpnEvi')
        self.Stubs[ObjectTypes.BGP_EVPN_EVI_RT] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.Channel, 'EvpnEviRt')
        self.Stubs[ObjectTypes.BGP_EVPN_IP_VRF] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.Channel, 'EvpnIpVrf')
        self.Stubs[ObjectTypes.BGP_EVPN_IP_VRF_RT] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.Channel, 'EvpnIpVrfRt')
        self.Stubs[ObjectTypes.STATIC_ROUTE] = ClientStub(cp_route_pb2_grpc.CPRouteSvcStub,
                                                        self.Channel, 'CPStaticRoute')
        return

    def CreateMsgReqTable(self):
        self.MsgReqs[ObjectTypes.DEVICE] = ClientModule(device_pb2, 'Device')
        self.MsgReqs[ObjectTypes.INTERFACE] = ClientModule(interface_pb2, 'Interface')
        self.MsgReqs[ObjectTypes.LMAPPING] = ClientModule(mapping_pb2, 'Mapping')
        self.MsgReqs[ObjectTypes.RMAPPING] = ClientModule(mapping_pb2, 'Mapping')
        self.MsgReqs[ObjectTypes.METER] = ClientModule(meter_pb2, 'Meter')
        self.MsgReqs[ObjectTypes.MIRROR] = ClientModule(mirror_pb2, 'MirrorSession')
        self.MsgReqs[ObjectTypes.NEXTHOP] = ClientModule(nh_pb2, 'Nexthop')
        self.MsgReqs[ObjectTypes.NEXTHOPGROUP] = ClientModule(nh_pb2, 'NhGroup')
        self.MsgReqs[ObjectTypes.ROUTE] = ClientModule(route_pb2, 'RouteTable')
        self.MsgReqs[ObjectTypes.POLICY] = ClientModule(policy_pb2, 'SecurityPolicy')
        self.MsgReqs[ObjectTypes.SECURITY_PROFILE] = ClientModule(policy_pb2, 'SecurityProfile')
        self.MsgReqs[ObjectTypes.SUBNET] = ClientModule(subnet_pb2, 'Subnet')
        self.MsgReqs[ObjectTypes.TAG] = ClientModule(tags_pb2, 'Tag')
        self.MsgReqs[ObjectTypes.TUNNEL] = ClientModule(tunnel_pb2, 'Tunnel')
        self.MsgReqs[ObjectTypes.VPC] = ClientModule(vpc_pb2, 'VPC')
        self.MsgReqs[ObjectTypes.VNIC] = ClientModule(vnic_pb2, 'Vnic')
        self.MsgReqs[ObjectTypes.DHCP_RELAY] = ClientModule(dhcp_pb2, 'DHCPPolicy')
        self.MsgReqs[ObjectTypes.DHCP_PROXY] = ClientModule(dhcp_pb2, 'DHCPPolicy')
        self.MsgReqs[ObjectTypes.NAT] = ClientModule(nat_pb2, 'NatPortBlock')
        self.MsgReqs[ObjectTypes.POLICER] = ClientModule(policer_pb2, 'Policer')
        self.MsgReqs[ObjectTypes.BGP] = ClientModule(bgp_pb2, 'BGP')
        self.MsgReqs[ObjectTypes.BGP_PEER] = ClientModule(bgp_pb2, 'BGPPeer')
        self.MsgReqs[ObjectTypes.BGP_PEER_AF] = ClientModule(bgp_pb2, 'BGPPeerAf')
        self.MsgReqs[ObjectTypes.BGP_EVPN_EVI] = ClientModule(evpn_pb2, 'EvpnEvi')
        self.MsgReqs[ObjectTypes.BGP_EVPN_EVI_RT] = ClientModule(evpn_pb2, 'EvpnEviRt')
        self.MsgReqs[ObjectTypes.BGP_EVPN_IP_VRF] = ClientModule(evpn_pb2, 'EvpnIpVrf')
        self.MsgReqs[ObjectTypes.BGP_EVPN_IP_VRF_RT] = ClientModule(evpn_pb2, 'EvpnIpVrfRt')
        self.MsgReqs[ObjectTypes.STATIC_ROUTE] = ClientModule(cp_route_pb2, 'CPStaticRoute')
        return

    def __create_restreq_table(self):
        self.RestReqs[ObjectTypes.VPC] = ClientRESTModule(self.IPAddr, "/api/vrfs/")
        self.RestReqs[ObjectTypes.ROUTE] = ClientRESTModule(self.IPAddr, "/api/route-tables/")
        self.RestReqs[ObjectTypes.SUBNET] = ClientRESTModule(self.IPAddr, "/api/networks/")
        self.RestReqs[ObjectTypes.BGP] = ClientRESTModule(self.IPAddr, "/api/routingconfigs/")
        self.RestReqs[ObjectTypes.INTERFACE] = ClientRESTModule(self.IPAddr, "/api/interfaces/")
        self.RestReqs[ObjectTypes.POLICY] = ClientRESTModule(self.IPAddr, "/api/security/policies/")
        self.RestReqs[ObjectTypes.DHCP_RELAY] = ClientRESTModule(self.IPAddr, "/api/ipam-policies/")
        return

    def CreateMsgRespTable(self):
        self.MsgResps[ObjectTypes.DEVICE] = ClientResponseModule(device_pb2, 'Device')
        self.MsgResps[ObjectTypes.INTERFACE] = ClientResponseModule(interface_pb2, 'Interface')
        self.MsgResps[ObjectTypes.LMAPPING] = ClientResponseModule(mapping_pb2, 'Mapping')
        self.MsgResps[ObjectTypes.RMAPPING] = ClientResponseModule(mapping_pb2, 'Mapping')
        self.MsgResps[ObjectTypes.METER] = ClientResponseModule(meter_pb2, 'Meter')
        self.MsgResps[ObjectTypes.MIRROR] = ClientResponseModule(mirror_pb2, 'MirrorSession')
        self.MsgResps[ObjectTypes.NEXTHOP] = ClientResponseModule(nh_pb2, 'Nexthop')
        self.MsgResps[ObjectTypes.NEXTHOPGROUP] = ClientResponseModule(nh_pb2, 'NhGroup')
        self.MsgResps[ObjectTypes.ROUTE] = ClientResponseModule(route_pb2, 'RouteTable')
        self.MsgResps[ObjectTypes.POLICY] = ClientResponseModule(policy_pb2, 'SecurityPolicy')
        self.MsgResps[ObjectTypes.SECURITY_PROFILE] = ClientResponseModule(policy_pb2, 'SecurityProfile')
        self.MsgResps[ObjectTypes.SUBNET] = ClientResponseModule(subnet_pb2, 'Subnet')
        self.MsgResps[ObjectTypes.TAG] = ClientResponseModule(tags_pb2, 'Tag')
        self.MsgResps[ObjectTypes.TUNNEL] = ClientResponseModule(tunnel_pb2, 'Tunnel')
        self.MsgResps[ObjectTypes.VPC] = ClientResponseModule(vpc_pb2, 'VPC')
        self.MsgResps[ObjectTypes.VNIC] = ClientResponseModule(vnic_pb2, 'Vnic')
        self.MsgResps[ObjectTypes.DHCP_RELAY] = ClientResponseModule(dhcp_pb2, 'DHCPPolicy')
        self.MsgResps[ObjectTypes.DHCP_PROXY] = ClientResponseModule(dhcp_pb2, 'DHCPPolicy')
        self.MsgResps[ObjectTypes.NAT] = ClientResponseModule(nat_pb2, 'NatPortBlock')
        self.MsgResps[ObjectTypes.POLICER] = ClientResponseModule(policer_pb2, 'Policer')
        self.MsgResps[ObjectTypes.BGP] = ClientResponseModule(bgp_pb2, 'BGP')
        self.MsgResps[ObjectTypes.BGP_PEER] = ClientResponseModule(bgp_pb2, 'BGPPeer')
        self.MsgResps[ObjectTypes.BGP_PEER_AF] = ClientResponseModule(bgp_pb2, 'BGPPeerAf')
        self.MsgResps[ObjectTypes.BGP_EVPN_EVI] = ClientResponseModule(evpn_pb2, 'EvpnEvi')
        self.MsgResps[ObjectTypes.BGP_EVPN_EVI_RT] = ClientResponseModule(evpn_pb2, 'EvpnEviRt')
        self.MsgResps[ObjectTypes.BGP_EVPN_IP_VRF] = ClientResponseModule(evpn_pb2, 'EvpnIpVrf')
        self.MsgResps[ObjectTypes.BGP_EVPN_IP_VRF_RT] = ClientResponseModule(evpn_pb2, 'EvpnIpVrfRt')
        self.MsgResps[ObjectTypes.STATIC_ROUTE] = ClientResponseModule(cp_route_pb2, 'CPStaticRoute')
        return

    def ParseServiceResponse(self, objtype, op, resp):
        msg = self.MsgResps[objtype].MsgResp(op)
        grpcmsg = msg()
        if (not resp) or (resp == -1):
            logger.info("No service response received in UDS")
        else:
            service_resp = types_pb2.ServiceResponseMessage()
            service_resp.ParseFromString(resp)
            val = service_resp.Response
            if val.Is(grpcmsg.DESCRIPTOR):
                val.Unpack(grpcmsg)
            logger.verbose(f"Received response over UDS: {service_resp}")
        return grpcmsg

    def SendServiceRequestMessage(self, oper, objs, objtype):
        service_op = ServiceOp.SERVICE_OP_NONE
        if oper == ApiOps.CREATE:
            service_op = ServiceOp.SERVICE_OP_CREATE
        elif oper == ApiOps.DELETE:
            service_op = ServiceOp.SERVICE_OP_DELETE
        elif oper == ApiOps.UPDATE:
            service_op = ServiceOp.SERVICE_OP_UPDATE
        elif oper == ApiOps.GET:
            service_op = ServiceOp.SERVICE_OP_READ

        service_req = types_pb2.ServiceRequestMessage()
        service_req.ConfigOp = service_op
        config_msg = service_req.ConfigMsg
        resps = list()
        server_address = SVC_SERVER_SOCKET_PATH

        def __triggerRequest(message):
            #Initialize unix socket
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            if not sock:
                logger.info("Failed to open unix domain socket")
                return -1
            try:
                ret = sock.connect_ex(server_address)
                if ret:
                    raise socket.error(ret)
                nbytes = message.SerializeToString()
                sock.sendall(nbytes)
                logger.verbose(f"Sent a UDS request with message: {message}")
                bytes_read = sock.recv(int(SOCK_BUF_LEN))
                if not bytes_read:
                    logger.info(f"No response received from remote socket")
            except socket.error as msg:
                logger.info(f"Received socket error while sending to {server_address}, err {msg}")
                bytes_read = -1
            finally:
                sock.close()
            return bytes_read

        for obj in objs:
            config_msg.Pack(obj)
            resp = __triggerRequest(service_req)
            service_resp = self.ParseServiceResponse(objtype, oper, resp)
            resps.append(service_resp)
        return resps

    def UdsSend(self, oper, objtype, objs):
        return self.SendServiceRequestMessage(oper, objs, objtype)

    def Create(self, objtype, objs):
        if GlobalOptions.netagent:
            if not self.RestReqs[objtype]:
                return # unsupported object
            return self.RestReqs[objtype].Create(objs)

        return super().Create(objtype, objs)

    def Delete(self, objtype, objs):
        if GlobalOptions.netagent:
            if not self.RestReqs[objtype]:
                return # unsupported object
            return self.RestReqs[objtype].Delete(objs)

        return super().Delete(objtype, objs)

    def Update(self, objtype, objs):
        if GlobalOptions.netagent:
            if not self.RestReqs[objtype]:
                return # unsupported object
            return self.RestReqs[objtype].Update(objs)

        return super().Update(objtype, objs)

    def GetHttp(self, objtype):
        if GlobalOptions.dryrun: return None
        if not GlobalOptions.netagent: return None
        if not self.RestReqs[objtype]:
            return None # unsupported object
        return self.RestReqs[objtype].Get()

    def GetRestURL(self):
        if not GlobalOptions.netagent: return None
        return f"https://{self.IPAddr}:{AgentPorts.DSCAGENTREST.value}"

    def Start(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.Stubs[objtype].Rpc(ApiOps.START, [obj])

    def Abort(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.Stubs[objtype].Rpc(ApiOps.ABORT, [obj])

    def Commit(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.Stubs[objtype].Rpc(ApiOps.COMMIT, [obj])

client = dict()
operdclient = dict()
penOperClient = dict()
def Init(node, ip=None):
    global client
    global operdclient
    global penOperClient
    client[node] = PdsAgentClient(ip)
    operdclient[node] = OperdClient(ip)
    penOperClient[node] = PenOperClient(ip)
