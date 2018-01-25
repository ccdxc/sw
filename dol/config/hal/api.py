#! /usr/bin/python3

import os
import types_pb2
import infra.common.defs    as defs
import config.hal.defs      as haldefs

import grpc

from infra.common.glopts  import GlobalOptions
from infra.common.logging import cfglogger
from mbt_signaling_client import SignalingClient

import types_pb2            as types_pb2
import vrf_pb2              as vrf_pb2
import interface_pb2        as interface_pb2
import l2segment_pb2        as l2segment_pb2
import endpoint_pb2         as endpoint_pb2
import session_pb2          as session_pb2
import nwsec_pb2            as nwsec_pb2
import nw_pb2               as nw_pb2
import telemetry_pb2        as telemetry_pb2
import tcp_proxy_cb_pb2     as tcpcb_pb2
import tls_proxy_cb_pb2     as tlscb_pb2
import descriptor_aol_pb2   as descriptor_aol_pb2
import wring_pb2            as wring_pb2
import acl_pb2              as acl_pb2
import qos_pb2              as qos_pb2
import proxy_pb2            as proxy_pb2
import ipseccb_pb2          as ipseccb_pb2
import l4lb_pb2             as l4lb_pb2
import crypto_keys_pb2      as crypto_keys_pb2
import rdma_pb2             as rdma_pb2
import cpucb_pb2            as cpucb_pb2
import rawrcb_pb2           as rawrcb_pb2
import rawccb_pb2           as rawccb_pb2
import proxyrcb_pb2         as proxyrcb_pb2
import proxyccb_pb2         as proxyccb_pb2
import multicast_pb2        as multicast_pb2
import barco_rings_pb2      as barco_rings_pb2

#import endpoint_pb2_grpc        as endpoint_pb2_grpc
#import l2segment_pb2_grpc       as l2segment_pb2_grpc
#import vrf_pb2_grpc          as vrf_pb2_grpc
#import interface_pb2_grpc       as interface_pb2_grpc
#import session_pb2_grpc         as session_pb2_grpc
#import nwsec_pb2_grpc           as nwsec_pb2_grpc
#import nw_pb2_grpc              as nw_pb2_grpc
#import telemetry_pb2_grpc       as telemetry_pb2_grpc
#import tcp_proxy_cb_pb2_grpc    as tcpcb_pb2_grpc
#import tls_proxy_cb_pb2_grpc    as tlscb_pb2_grpc
#import descriptor_aol_pb2_grpc  as descriptor_aol_pb2_grpc
#import wring_pb2_grpc           as wring_pb2_grpc
#import acl_pb2_grpc             as acl_pb2_grpc
#import qos_pb2_grpc             as qos_pb2_grpc
#import proxy_pb2_grpc           as proxy_pb2_grpc
#import ipseccb_pb2_grpc         as ipseccb_pb2_grpc
#import l4lb_pb2_grpc            as l4lb_pb2_grpc
#import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc
#import rdma_pb2_grpc            as rdma_pb2_grpc
#import cpucb_pb2_grpc           as cpucb_pb2_grpc
#import rawrcb_pb2_grpc          as rawrcb_pb2_grpc
#import rawccb_pb2_grpc          as rawccb_pb2_grpc

HAL_MAX_BATCH_SIZE = 64

HalChannel = None
class HalInterfaceSegmentAssociation:
    def __init__(self, intf, seg):
        self.intf   = intf
        self.seg    = seg
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.if_key_handle.interface_id = self.intf.id
        req_spec.l2segment_key_or_handle.segment_id = self.seg.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("- Intf:%s <--> Seg:%s. Status = %s)" %\
                       (self.intf.GID(), self.seg.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

def __process_response(resp_msg, req_msg, req_objs, respcb):
    num_req_specs = len(req_msg.request)
    num_resp_specs = len(resp_msg.response)
    if num_req_specs != num_resp_specs:
        cfglogger.error(" - Bad # of resp_specs:%d Expected:%d" %\
                        (num_resp_specs, num_req_specs))
        assert(0)

    for idx in range(len(req_msg.request)):
        req_spec = req_msg.request[idx]
        resp_spec = resp_msg.response[idx]
        req_obj = req_objs[idx]
        getattr(req_obj, respcb)(req_spec, resp_spec)
        if resp_spec.api_status != types_pb2.API_STATUS_OK:
            cfglogger.error(" HAL Returned API Status:%d" % (resp_spec.api_status))
            assert(0)
    return

def __hal_api_handler(objs, reqmsg_class, api, reqcb, respcb):
    req_msg = reqmsg_class()
    req_objs = []
    count = 0
    for obj in objs:
        req_spec = req_msg.request.add()
        getattr(obj,reqcb)(req_spec)
        req_objs.append(obj)

        count += 1
        if count >= HAL_MAX_BATCH_SIZE:
            resp_msg = api(req_msg)
            __process_response(resp_msg, req_msg, req_objs, respcb)
            req_msg = reqmsg_class()
            req_objs = []
            count = 0

    if count != 0:
        resp_msg = api(req_msg)
        __process_response(resp_msg, req_msg, req_objs, respcb)
    return

def __config(objs, reqmsg_class, config_method):
    return __hal_api_handler(objs, reqmsg_class, config_method,\
                            "PrepareHALRequestSpec", "ProcessHALResponse")


def __get(objs, reqmsg_class, config_method):
    return __hal_api_handler(objs, reqmsg_class, config_method,\
                            "PrepareHALGetRequestSpec", "ProcessHALGetResponse")

def __delete(objs, reqmsg_class, config_method):
    return __hal_api_handler(objs, reqmsg_class, config_method,\
                            "PrepareHALDeleteRequestSpec", "ProcessHALDeleteResponse")

def init():
    global HalChannel
    if IsHalDisabled(): return

    if 'MBT_GRPC_PORT' in os.environ: # If MBT toggle mode is enabled
        assert GlobalOptions.mbt
        port = os.environ['MBT_GRPC_PORT']
    elif 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    cfglogger.info("Creating GRPC channel to HAL on port %s" %(port))
    server = 'localhost:' + port
    HalChannel = grpc.insecure_channel(server)
    cfglogger.info("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(HalChannel).result()
    cfglogger.info("Connected to HAL!")
    if GlobalOptions.mbt:
        SignalingClient.Connect()
        cfglogger.info("Connected to the Model based tester")
    return

def IsHalDisabled():
    return GlobalOptions.no_hal or GlobalOptions.dryrun

def ConfigureLifs(objlist, update = False):
    if IsHalDisabled(): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    api = stub.LifUpdate if update else stub.LifCreate
    msg = interface_pb2.LifRequestMsg
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetLifs(objlist):
    if IsHalDisabled():
        return
    stub = interface_pb2.InterfaceStub(HalChannel)
    __get(objlist, interface_pb2.LifGetRequestMsg,
          stub.LifGet)
    return

def ConfigureInterfaces(objlist, update = False):
    if IsHalDisabled(): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    msg = interface_pb2.InterfaceRequestMsg
    api = stub.InterfaceUpdate if update else stub.InterfaceCreate
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetInterfaces(objlist):
    if IsHalDisabled():
        return
    stub = interface_pb2.InterfaceStub(HalChannel)
    __get(objlist, interface_pb2.InterfaceGetRequestMsg,
          stub.InterfaceGet)
    return

def ConfigureInterfaceSegmentAssociations(intfs, segs):
    if IsHalDisabled(): return
    stub = interface_pb2.InterfaceStub(HalChannel)

    objs = []
    for intf in intfs:
        for seg in segs:
            intf_seg_assoc = HalInterfaceSegmentAssociation(intf, seg)
            objs.append(intf_seg_assoc)
    __config(objs, interface_pb2.InterfaceL2SegmentRequestMsg,
             stub.AddL2SegmentOnUplink)
    return


def ConfigureTenants(objlist):
    if IsHalDisabled(): return
    stub = vrf_pb2.VrfStub(HalChannel)
    msg = vrf_pb2.VrfRequestMsg
    __config(objlist, msg,
             stub.VrfCreate)
    if GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetTenants(objlist):
    if IsHalDisabled():
        return
    stub = vrf_pb2.VrfStub(HalChannel)
    __get(objlist, vrf_pb2.VrfGetRequestMsg,
          stub.VrfGet)
    return

def ConfigureTcpCbs(objlist):
    if IsHalDisabled(): return
    stub = tcpcb_pb2.TcpCbStub(HalChannel)
    __config(objlist, tcpcb_pb2.TcpCbRequestMsg,
             stub.TcpCbCreate)
    return

def ConfigureIpsecCbs(objlist):
    if IsHalDisabled(): return
    stub = ipseccb_pb2.IpsecCbStub(HalChannel)
    __config(objlist, ipseccb_pb2.IpsecCbRequestMsg,
             stub.IpsecCbCreate)
    return

def UpdateTcpCbs(objlist):
    if IsHalDisabled(): return
    stub = tcpcb_pb2.TcpCbStub(HalChannel)
    __config(objlist, tcpcb_pb2.TcpCbRequestMsg,
             stub.TcpCbUpdate)
    return

def UpdateTlsCbs(objlist):
    if IsHalDisabled(): return
    stub = tlscb_pb2.TlsCbStub(HalChannel)
    __config(objlist, tlscb_pb2.TlsCbRequestMsg,
             stub.TlsCbUpdate)
    return

def UpdateIpsecCbs(objlist):
    if IsHalDisabled(): return
    stub = ipseccb_pb2.IpsecCbStub(HalChannel)
    __config(objlist, ipseccb_pb2.IpsecCbRequestMsg,
             stub.IpsecCbUpdate)
    return

def ConfigureCpuCbs(objlist):
    if IsHalDisabled(): return
    stub = cpucb_pb2.CpuCbStub(HalChannel)
    __config(objlist, cpucb_pb2.CpuCbRequestMsg,
             stub.CpuCbCreate)
    return

def UpdateCpuCbs(objlist):
    if IsHalDisabled(): return
    stub = cpucb_pb2.CpuCbStub(HalChannel)
    __config(objlist, cpucb_pb2.CpuCbRequestMsg,
             stub.CpuCbUpdate)
    return

def GetCpuCbs(objlist):
    if IsHalDisabled(): return
    stub = cpucb_pb2.CpuCbStub(HalChannel)
    __config(objlist, cpucb_pb2.CpuCbGetRequestMsg,
             stub.CpuCbGet)
    return

def ConfigureRawrCbs(objlist):
    if IsHalDisabled(): return
    stub = rawrcb_pb2.RawrCbStub(HalChannel)
    __config(objlist, rawrcb_pb2.RawrCbRequestMsg,
             stub.RawrCbCreate)
    return

def UpdateRawrCbs(objlist):
    if IsHalDisabled(): return
    stub = rawrcb_pb2.RawrCbStub(HalChannel)
    __config(objlist, rawrcb_pb2.RawrCbRequestMsg,
             stub.RawrCbUpdate)
    return

def GetRawrCbs(objlist):
    if IsHalDisabled(): return
    stub = rawrcb_pb2.RawrCbStub(HalChannel)
    __config(objlist, rawrcb_pb2.RawrCbGetRequestMsg,
             stub.RawrCbGet)
    return

def ConfigureProxyrCbs(objlist):
    if IsHalDisabled(): return
    stub = proxyrcb_pb2.ProxyrCbStub(HalChannel)
    __config(objlist, proxyrcb_pb2.ProxyrCbRequestMsg,
             stub.ProxyrCbCreate)
    return

def UpdateProxyrCbs(objlist):
    if IsHalDisabled(): return
    stub = proxyrcb_pb2.ProxyrCbStub(HalChannel)
    __config(objlist, proxyrcb_pb2.ProxyrCbRequestMsg,
             stub.ProxyrCbUpdate)
    return

def GetProxyrCbs(objlist):
    if IsHalDisabled(): return
    stub = proxyrcb_pb2.ProxyrCbStub(HalChannel)
    __config(objlist, proxyrcb_pb2.ProxyrCbGetRequestMsg,
             stub.ProxyrCbGet)
    return

def ConfigureRawcCbs(objlist):
    if IsHalDisabled(): return
    stub = rawccb_pb2.RawcCbStub(HalChannel)
    __config(objlist, rawccb_pb2.RawcCbRequestMsg,
             stub.RawcCbCreate)
    return

def UpdateRawcCbs(objlist):
    if IsHalDisabled(): return
    stub = rawccb_pb2.RawcCbStub(HalChannel)
    __config(objlist, rawccb_pb2.RawcCbRequestMsg,
             stub.RawcCbUpdate)
    return

def GetRawcCbs(objlist):
    if IsHalDisabled(): return
    stub = rawccb_pb2.RawcCbStub(HalChannel)
    __config(objlist, rawccb_pb2.RawcCbGetRequestMsg,
             stub.RawcCbGet)
    return

def ConfigureProxycCbs(objlist):
    if IsHalDisabled(): return
    stub = proxyccb_pb2.ProxycCbStub(HalChannel)
    __config(objlist, proxyccb_pb2.ProxycCbRequestMsg,
             stub.ProxycCbCreate)
    return

def UpdateProxycCbs(objlist):
    if IsHalDisabled(): return
    stub = proxyccb_pb2.ProxycCbStub(HalChannel)
    __config(objlist, proxyccb_pb2.ProxycCbRequestMsg,
             stub.ProxycCbUpdate)
    return

def GetProxycCbs(objlist):
    if IsHalDisabled(): return
    stub = proxyccb_pb2.ProxycCbStub(HalChannel)
    __config(objlist, proxyccb_pb2.ProxycCbGetRequestMsg,
             stub.ProxycCbGet)
    return

def GetTcpCbs(objlist):
    if IsHalDisabled(): return
    stub = tcpcb_pb2.TcpCbStub(HalChannel)
    __config(objlist, tcpcb_pb2.TcpCbGetRequestMsg,
             stub.TcpCbGet)
    return

def GetIpsecCbs(objlist):
    if IsHalDisabled(): return
    stub = ipseccb_pb2.IpsecCbStub(HalChannel)
    __config(objlist, ipseccb_pb2.IpsecCbGetRequestMsg,
             stub.IpsecCbGet)
    return

def GetTlsCbs(objlist):
    if IsHalDisabled(): return
    stub = tlscb_pb2.TlsCbStub(HalChannel)
    __config(objlist, tlscb_pb2.TlsCbGetRequestMsg,
             stub.TlsCbGet)
    return

def ConfigureProxyService(objlist):
    if IsHalDisabled(): return
    stub = proxy_pb2.ProxyStub(HalChannel)
    __config(objlist, proxy_pb2.ProxyRequestMsg,
             stub.ProxyEnable)
    return

def ConfigureTlsCbs(objlist):
    if IsHalDisabled(): return
    stub = tlscb_pb2.TlsCbStub(HalChannel)
    __config(objlist, tlscb_pb2.TlsCbRequestMsg,
             stub.TlsCbCreate)
    return

def ConfigureSegments(objlist):
    if IsHalDisabled(): return
    stub = l2segment_pb2.L2SegmentStub(HalChannel)
    msg = l2segment_pb2.L2SegmentRequestMsg
    __config(objlist, msg,
             stub.L2SegmentCreate)
    if GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetSegments(objlist):
    if IsHalDisabled():
        return
    stub = l2segment_pb2.L2SegmentStub(HalChannel)
    __get(objlist, l2segment_pb2.L2SegmentGetRequestMsg,
          stub.L2SegmentGet)
    return

def ConfigureEndpoints(objlist):
    if IsHalDisabled(): return
    stub = endpoint_pb2.EndpointStub(HalChannel)
    __config(objlist, endpoint_pb2.EndpointRequestMsg,
             stub.EndpointCreate)
    return

def GetEndpoints(objlist):
    if IsHalDisabled():
        return
    stub = endpoint_pb2.EndpointStub(HalChannel)
    __get(objlist, endpoint_pb2.EndpointGetRequestMsg,
          stub.EndpointGet)
    return

def ConfigureSessions(objlist):
    if IsHalDisabled(): return
    stub = session_pb2.SessionStub(HalChannel)
    __config(objlist, session_pb2.SessionRequestMsg,
             stub.SessionCreate)
    return

def GetSessions(objlist):
    if IsHalDisabled():
        return
    stub = session_pb2.SessionStub(HalChannel)
    __get(objlist, session_pb2.SessionGetRequestMsg,
          stub.SessionGet)
    return

def ConfigureLifsForObjects(objlist):
    lif_objlist = []
    for obj in objlist:
        lif_objlist.append(obj.lif)
    cfglogger.info("Configuring %d LIFs." % len(lif_objlist))
    ConfigureLifs(lif_objlist)
    return

def ConfigureSecurityProfiles(objlist, update = False):
    if IsHalDisabled(): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    api = stub.SecurityProfileCreate
    msg = nwsec_pb2.SecurityProfileRequestMsg
    if update: api = stub.SecurityProfileUpdate
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetSecurityProfiles(objlist):
    if IsHalDisabled():
        return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    __get(objlist, nwsec_pb2.SecurityProfileGetRequestMsg,
          stub.SecurityProfileGet)
    return

def ConfigureSecurityGroups(objlist, update = False):
    if IsHalDisabled(): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    api = stub.SecurityGroupCreate
    msg = nwsec_pb2.SecurityGroupRequestMsg
    if update: api = stub.SecurityGroupUpdate
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def ConfigureDosPolicies(objlist, update = False):
    if IsHalDisabled(): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    api = stub.DoSPolicyCreate
    if update: api = stub.DoSPolicyUpdate
    __config(objlist, nwsec_pb2.DoSPolicyRequestMsg, api)
    return

def GetSecurityGroups(objlist):
    if IsHalDisabled():
        return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    __get(objlist, nwsec_pb2.SecurityGroupGetRequestMsg,
          stub.SecurityGroupGet)

def ConfigureSecurityGroupPolicies(objlist, update = False):
    if IsHalDisabled(): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    api = stub.SecurityGroupPolicyCreate
    msg = nwsec_pb2.SecurityGroupPolicyRequestMsg
    if update: api = stub.SecurityGroupPolicyUpdate
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetSecurityGroupPolicies(objlist):
    if IsHalDisabled():
        return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    __get(objlist, nwsec_pb2.SecurityGroupPolicyGetRequestMsg,
          stub.SecurityGroupPolicyGet)
    return

def ConfigureSpanSessions(objlist):
    if IsHalDisabled(): return
    stub = telemetry_pb2.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.MirrorSessionConfigMsg,
             stub.MirrorSessionCreate)
    return

def DeleteSpanSessions(objlist):
    if IsHalDisabled(): return
    stub = telemetry_pb2.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.MirrorSessionDeleteMsg,
             stub.MirrorSessionDelete)
    return

def ConfigureCollectors(objlist):
    if IsHalDisabled(): return
    stub = telemetry_pb2.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.CollectorConfigMsg,
             stub.CollectorCreate)
    return


def ConfigureNetworks(objlist):
    if IsHalDisabled(): return
    stub = nw_pb2.NetworkStub(HalChannel)
    msg = nw_pb2.NetworkRequestMsg
    __config(objlist, msg, stub.NetworkCreate)
    if GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetNetworks(objlist):
    if IsHalDisabled():
        return
    stub = nw_pb2.NetworkStub(HalChannel)
    __get(objlist, nw_pb2.NetworkGetRequestMsg,
          stub.NetworkGet)
    return

def GetDscrAolObjectState(objlist):
    if IsHalDisabled(): return
    stub = descriptor_aol_pb2.DescrAolStub(HalChannel)
    __config(objlist, descriptor_aol_pb2.DescrAolRequestMsg,
             stub.DescrAolGet)
    return

def GetRingEntries(objlist):
    if IsHalDisabled(): return
    stub = wring_pb2.WRingStub(HalChannel)
    __config(objlist, wring_pb2.WRingGetEntriesRequestMsg,
             stub.WRingGetEntries)
    return

def GetRingMeta(objlist):
    if IsHalDisabled(): return
    stub = wring_pb2.WRingStub(HalChannel)
    __get(objlist, wring_pb2.WRingRequestMsg,
             stub.WRingGetMeta)
    return

def SetRingMeta(objlist):
    if IsHalDisabled(): return
    stub = wring_pb2.WRingStub(HalChannel)
    __config(objlist, wring_pb2.WRingRequestMsg,
             stub.WRingSetMeta)
    return

def ConfigureAcls(objlist, update = False):
    if IsHalDisabled(): return
    stub = acl_pb2.AclStub(HalChannel)
    api = stub.AclUpdate if update else stub.AclCreate
    msg = acl_pb2.AclRequestMsg
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def DeleteAcls(objlist):
    if IsHalDisabled(): return
    stub = acl_pb2.AclStub(HalChannel)
    __delete(objlist, acl_pb2.AclDeleteRequestMsg,
             stub.AclDelete)
    return


def ConfigureL4LbBackends(objlist):
    if IsHalDisabled(): return
    stub = l4lb_pb2.L4LbStub(HalChannel)
    __config(objlist, l4lb_pb2.L4LbBackendRequestMsg,
             stub.L4LbBackendCreate)
    return

def ConfigureL4LbServices(objlist):
    if IsHalDisabled(): return
    stub = l4lb_pb2.L4LbStub(HalChannel)
    __config(objlist, l4lb_pb2.L4LbServiceRequestMsg,
             stub.L4LbServiceCreate)
    return

def GetCryptoKey(objlist):
    if IsHalDisabled(): return
    stub = crypto_keys_pb2.CryptoKeyStub(HalChannel)
    __config(objlist, crypto_keys_pb2.CryptoKeyCreateRequestMsg,
             stub.CryptoKeyCreate)
    return

def UpdateCryptoKeys(objlist):
    if IsHalDisabled(): return
    stub = crypto_keys_pb2.CryptoKeyStub(HalChannel)
    __config(objlist, crypto_keys_pb2.CryptoKeyUpdateRequestMsg,
             stub.CryptoKeyUpdate)
    return

def ConfigureQps(objlist):
    if IsHalDisabled(): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaQpRequestMsg,
             stub.RdmaQpCreate)
    return

def ModifyQps(objlist):
    if IsHalDisabled(): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaQpUpdateRequestMsg,
             stub.RdmaQpUpdate)

def ConfigureAhs(objlist):
    if IsHalDisabled(): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaAhRequestMsg,
             stub.RdmaAhCreate)
    return

def ConfigureCqs(objlist):
    if IsHalDisabled(): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaCqRequestMsg,
             stub.RdmaCqCreate)
    return

def ConfigureEqs(objlist):
    if IsHalDisabled(): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaEqRequestMsg,
             stub.RdmaEqCreate)
    return

def ConfigureMrs(objlist):
    if IsHalDisabled(): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaMemRegRequestMsg,
             stub.RdmaMemReg)
    return

def ConfigureProxyCbService(objlist):
    if IsHalDisabled(): return
    stub = proxy_pb2.ProxyStub(HalChannel)
    __config(objlist, proxy_pb2.ProxyFlowConfigRequestMsg,
             stub.ProxyFlowConfig)
    return

def GetQidProxycbGetFlowInfo(objlist):
    if IsHalDisabled(): return
    stub = proxy_pb2.ProxyStub(HalChannel)
    __config(objlist, proxy_pb2.ProxyGetFlowInfoRequestMsg,
             stub.ProxyGetFlowInfo)
    return

def ConfigureMulticastGroups(objlist, update = False):
    if IsHalDisabled(): return
    stub = multicast_pb2.MulticastStub(HalChannel)
    api =  stub.MulticastEntryUpdate if update else stub.MulticastEntryCreate
    __config(objlist, multicast_pb2.MulticastEntryRequestMsg, api)
    return

def GetBarcoRingEntries(objlist):
    if IsHalDisabled(): return
    stub = barco_rings_pb2.BarcoRingsStub(HalChannel)
    __config(objlist, barco_rings_pb2.BarcoGetReqDescrEntryRequestMsg,
             stub.BarcoGetReqDescrEntry)
    return

def GetBarcoRingMeta(objlist):
    if IsHalDisabled(): return
    stub = barco_rings_pb2.BarcoRingsStub(HalChannel)
    __config(objlist, barco_rings_pb2.BarcoGetRingMetaRequestMsg,
             stub.BarcoGetRingMeta)
    return

def ConfigureQosClass(objlist):
    if IsHalDisabled(): return
    stub = qos_pb2.QOSStub(HalChannel)
    __config(objlist, qos_pb2.QosClassRequestMsg,
             stub.QosClassCreate)
    return

