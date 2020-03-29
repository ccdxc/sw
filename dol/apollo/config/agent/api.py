#! /usr/bin/python3
import os
import pdb
import grpc
import enum
import requests
import time
import urllib3

import oper_pb2_grpc as oper_pb2_grpc
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

from infra.common.glopts  import GlobalOptions
from infra.common.logging import logger

# Connection timeout - 3mins
MAX_CONNECT_TIMEOUT = 180
# RPC Timeout - 20mins
MAX_GRPC_WAIT = 1200
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
    OPER = 30
    SECURITY_PROFILE = 31
    MAX = 32

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

class ClientRESTModule:
    def __init__(self, ip, uri):
        self.url = "https://" + ip + ":8888" + uri
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
        res = continuation(call_info, req)
        if not isinstance(res, grpc.RpcError):
            # return if this is not error case
            return res
        # get the error code
        rc = res.code()
        if rc not in self.__status_codes2retry:
            # return if there is no need to retry for this error code
            return res
        # need to retry for this error
        return None

    def __intercept_rpc(self, continuation, call_info, req):
        for __retry in range(self.__max_retries):
            res = self.__continue_rpc(continuation, call_info, req)
            if res is not None:
                # return in case of success / uninterested responses
                return res
            logger.info(f"sleeping for {self.__backoff_timeout}s before rpc retry")
            time.sleep(self.__backoff_timeout)
        logger.error("Max rpc retries elapsed")
        return res

    def intercept_unary_unary(self, continuation, client_call_details, request):
        # Intercepts a unary-unary invocation asynchronously
        return self.__intercept_rpc(continuation, client_call_details, request)

    def intercept_stream_unary(self, continuation, client_call_details, request_iterator):
        # Intercepts a stream-unary invocation asynchronously
        return self.__intercept_rpc(continuation, client_call_details, request_iterator)

class ApolloAgentClientRequest:
    def __init__(self, stub, req,):
        self.__stub = stub
        return

class ApolloAgentClient:
    rpcinterceptors = (RetryOnFailureInterceptors(),)

    def __init__(self, ip = None):
        self.__channel = None
        self.__stubs = [None] * ObjectTypes.MAX
        self.__msgreqs = [None] * ObjectTypes.MAX
        self.__restreqs = [None] * ObjectTypes.MAX
        if ip == None:
            self.agentip = self.__get_agent_ip()
        else:
            self.agentip = ip
        self.agentport = self.__get_agent_port()
        self.__create_msgreq_table()
        self.__create_channel()
        if not self.__connect():
            assert(0)
        self.__create_stubs()
        self.__create_restreq_table()
        urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
        return

    def __get_agent_port(self):
        try:
            port = os.environ['AGENT_GRPC_PORT']
        except:
            port = '50054'
        return port;

    def __get_agent_ip(self):
        try:
            agentip = os.environ['AGENT_GRPC_IP']
        except:
            agentip = 'localhost'
        return agentip

    def __create_channel(self):
        endpoint = "%s:%s" % (self.agentip, self.agentport)
        logger.info(f"Agent info %s" % endpoint)
        channel = grpc.insecure_channel(endpoint)
        self.__channel = grpc.intercept_channel(channel, *ApolloAgentClient.rpcinterceptors)

    def __connect(self):
        if GlobalOptions.dryrun: return True
        logger.info("Waiting for Agent to be ready ...")
        try:
            grpc.channel_ready_future(self.__channel).result(MAX_CONNECT_TIMEOUT)
        except:
            logger.error("Error in establishing connection to Agent")
            return False
        logger.info("Connected to Agent!")
        return True

    def __create_stubs(self):
        if GlobalOptions.dryrun: return
        self.__stubs[ObjectTypes.OPER] = ClientStub(oper_pb2_grpc.OperSvcStub,
                                                    self.__channel, 'Oper')
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
        self.__stubs[ObjectTypes.LMAPPING] = ClientStub(mapping_pb2_grpc.MappingSvcStub,
                                                      self.__channel, 'Mapping')
        self.__stubs[ObjectTypes.RMAPPING] = ClientStub(mapping_pb2_grpc.MappingSvcStub,
                                                      self.__channel, 'Mapping')
        self.__stubs[ObjectTypes.ROUTE] = ClientStub(route_pb2_grpc.RouteSvcStub,
                                                      self.__channel, 'RouteTable')
        self.__stubs[ObjectTypes.POLICY] = ClientStub(policy_pb2_grpc.SecurityPolicySvcStub,
                                                      self.__channel, 'SecurityPolicy')
        self.__stubs[ObjectTypes.SECURITY_PROFILE] = ClientStub(policy_pb2_grpc.SecurityPolicySvcStub,
                                                      self.__channel, 'SecurityProfile')
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
        self.__stubs[ObjectTypes.DHCP_RELAY] = ClientStub(dhcp_pb2_grpc.DHCPSvcStub,
                                                      self.__channel, 'DHCPPolicy')
        self.__stubs[ObjectTypes.NAT] = ClientStub(nat_pb2_grpc.NatSvcStub,
                                                      self.__channel, 'NatPortBlock')
        self.__stubs[ObjectTypes.POLICER] = ClientStub(policer_pb2_grpc.PolicerSvcStub,
                                                      self.__channel, 'Policer')
        self.__stubs[ObjectTypes.BGP] = ClientStub(bgp_pb2_grpc.BGPSvcStub,
                                                   self.__channel, 'BGP')
        self.__stubs[ObjectTypes.BGP_PEER] = ClientStub(bgp_pb2_grpc.BGPSvcStub,
                                                        self.__channel, 'BGPPeer')
        self.__stubs[ObjectTypes.BGP_PEER_AF] = ClientStub(bgp_pb2_grpc.BGPSvcStub,
                                                           self.__channel, 'BGPPeerAf')
        self.__stubs[ObjectTypes.BGP_EVPN_EVI] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.__channel, 'EvpnEvi')
        self.__stubs[ObjectTypes.BGP_EVPN_EVI_RT] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.__channel, 'EvpnEviRt')
        self.__stubs[ObjectTypes.BGP_EVPN_IP_VRF] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.__channel, 'EvpnIpVrf')
        self.__stubs[ObjectTypes.BGP_EVPN_IP_VRF_RT] = ClientStub(evpn_pb2_grpc.EvpnSvcStub,
                                                    self.__channel, 'EvpnIpVrfRt')
        self.__stubs[ObjectTypes.STATIC_ROUTE] = ClientStub(cp_route_pb2_grpc.CPRouteSvcStub,
                                                        self.__channel, 'CPStaticRoute')
        return

    def __create_msgreq_table(self):
        self.__msgreqs[ObjectTypes.DEVICE] = ClientModule(device_pb2, 'Device')
        self.__msgreqs[ObjectTypes.INTERFACE] = ClientModule(interface_pb2, 'Interface')
        self.__msgreqs[ObjectTypes.LMAPPING] = ClientModule(mapping_pb2, 'Mapping')
        self.__msgreqs[ObjectTypes.RMAPPING] = ClientModule(mapping_pb2, 'Mapping')
        self.__msgreqs[ObjectTypes.METER] = ClientModule(meter_pb2, 'Meter')
        self.__msgreqs[ObjectTypes.MIRROR] = ClientModule(mirror_pb2, 'MirrorSession')
        self.__msgreqs[ObjectTypes.NEXTHOP] = ClientModule(nh_pb2, 'Nexthop')
        self.__msgreqs[ObjectTypes.NEXTHOPGROUP] = ClientModule(nh_pb2, 'NhGroup')
        self.__msgreqs[ObjectTypes.ROUTE] = ClientModule(route_pb2, 'RouteTable')
        self.__msgreqs[ObjectTypes.POLICY] = ClientModule(policy_pb2, 'SecurityPolicy')
        self.__msgreqs[ObjectTypes.SECURITY_PROFILE] = ClientModule(policy_pb2, 'SecurityProfile')
        self.__msgreqs[ObjectTypes.SUBNET] = ClientModule(subnet_pb2, 'Subnet')
        self.__msgreqs[ObjectTypes.TAG] = ClientModule(tags_pb2, 'Tag')
        self.__msgreqs[ObjectTypes.TUNNEL] = ClientModule(tunnel_pb2, 'Tunnel')
        self.__msgreqs[ObjectTypes.VPC] = ClientModule(vpc_pb2, 'VPC')
        self.__msgreqs[ObjectTypes.VNIC] = ClientModule(vnic_pb2, 'Vnic')
        self.__msgreqs[ObjectTypes.DHCP_RELAY] = ClientModule(dhcp_pb2, 'DHCPPolicy')
        self.__msgreqs[ObjectTypes.NAT] = ClientModule(nat_pb2, 'NatPortBlock')
        self.__msgreqs[ObjectTypes.POLICER] = ClientModule(policer_pb2, 'Policer')
        self.__msgreqs[ObjectTypes.BGP] = ClientModule(bgp_pb2, 'BGP')
        self.__msgreqs[ObjectTypes.BGP_PEER] = ClientModule(bgp_pb2, 'BGPPeer')
        self.__msgreqs[ObjectTypes.BGP_PEER_AF] = ClientModule(bgp_pb2, 'BGPPeerAf')
        self.__msgreqs[ObjectTypes.BGP_EVPN_EVI] = ClientModule(evpn_pb2, 'EvpnEvi')
        self.__msgreqs[ObjectTypes.BGP_EVPN_EVI_RT] = ClientModule(evpn_pb2, 'EvpnEviRt')
        self.__msgreqs[ObjectTypes.BGP_EVPN_IP_VRF] = ClientModule(evpn_pb2, 'EvpnIpVrf')
        self.__msgreqs[ObjectTypes.BGP_EVPN_IP_VRF_RT] = ClientModule(evpn_pb2, 'EvpnIpVrfRt')
        self.__msgreqs[ObjectTypes.STATIC_ROUTE] = ClientModule(cp_route_pb2, 'CPStaticRoute')
        return

    def __create_restreq_table(self):
        self.__restreqs[ObjectTypes.VPC] = ClientRESTModule(self.agentip, "/api/vrfs/")
        self.__restreqs[ObjectTypes.ROUTE] = ClientRESTModule(self.agentip, "/api/route-tables/")
        self.__restreqs[ObjectTypes.SUBNET] = ClientRESTModule(self.agentip, "/api/networks/")
        self.__restreqs[ObjectTypes.BGP] = ClientRESTModule(self.agentip, "/api/routingconfigs/")
        self.__restreqs[ObjectTypes.INTERFACE] = ClientRESTModule(self.agentip, "/api/interfaces/")
        self.__restreqs[ObjectTypes.POLICY] = ClientRESTModule(self.agentip, "/api/security/policies/")
        self.__restreqs[ObjectTypes.DHCP_RELAY] = ClientRESTModule(self.agentip, "/api/ipam-policies/")
        return

    def GetGRPCMsgReq(self, objtype, op):
        return self.__msgreqs[objtype].MsgReq(op)

    def Create(self, objtype, objs):
        if GlobalOptions.netagent:
            if not self.__restreqs[objtype]:
                return # unsupported object
            return self.__restreqs[objtype].Create(objs)
        if GlobalOptions.dryrun: return True
        return self.__stubs[objtype].Rpc(ApiOps.CREATE, objs)

    def Delete(self, objtype, objs):
        if GlobalOptions.netagent:
            if not self.__restreqs[objtype]:
                return # unsupported object
            return self.__restreqs[objtype].Delete(objs)
        if GlobalOptions.dryrun: return True
        return self.__stubs[objtype].Rpc(ApiOps.DELETE, objs)

    def Update(self, objtype, objs):
        if GlobalOptions.netagent:
            if not self.__restreqs[objtype]:
                return # unsupported object
            return self.__restreqs[objtype].Update(objs)
        if GlobalOptions.dryrun: return True
        return self.__stubs[objtype].Rpc(ApiOps.UPDATE, objs)

    def GetGrpc(self, objtype, objs):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.GET, objs)

    def GetHttp(self, objtype):
        if GlobalOptions.dryrun: return None
        if not self.__restreqs[objtype]:
            return None # unsupported object
        return self.__restreqs[objtype].Get()

    def Get(self, objtype, objs):
        #if GlobalOptions.netagent:
            #return self.GetHttp(objtype)
        # default to grpc for now
        return self.GetGrpc(objtype, objs)

    def Request(self, objtype, rpcname, objs):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].DoRpc(rpcname, objs)

    def Start(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.START, [ obj ])

    def Abort(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.ABORT, [ obj ])

    def Commit(self, objtype, obj):
        if GlobalOptions.dryrun: return
        return self.__stubs[objtype].Rpc(ApiOps.COMMIT, [ obj ])

client = dict()
def Init(node, ip=None):
    global client
    client[node] = ApolloAgentClient(ip)
