#! /usr/bin/python3

import os
import types_pb2
import infra.common.defs    as defs
import config.hal.defs      as haldefs

import grpc

from infra.common.glopts import GlobalOptions
from infra.common.logging import cfglogger

import types_pb2            as types_pb2
import tenant_pb2           as tenant_pb2
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
import proxy_pb2            as proxy_pb2
import ipseccb_pb2          as ipseccb_pb2
import l4lb_pb2             as l4lb_pb2
import crypto_keys_pb2      as crypto_keys_pb2
import rdma_pb2             as rdma_pb2
import cpucb_pb2            as cpucb_pb2

#import endpoint_pb2_grpc        as endpoint_pb2_grpc
#import l2segment_pb2_grpc       as l2segment_pb2_grpc
#import tenant_pb2_grpc          as tenant_pb2_grpc
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
#import proxy_pb2_grpc           as proxy_pb2_grpc
#import ipseccb_pb2_grpc         as ipseccb_pb2_grpc
#import l4lb_pb2_grpc            as l4lb_pb2_grpc
#import crypto_keys_pb2_grpc     as crypto_keys_pb2_grpc
#import rdma_pb2_grpc            as rdma_pb2_grpc
#import cpucb_pb2_grpc           as cpucb_pb2_grpc

HAL_MAX_BATCH_SIZE = 64

HalChannel = None
class HalInterfaceSegmentAssociation:
    def __init__(self, intf, seg):
        self.intf   = intf
        self.seg    = seg
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.if_key_handle.if_handle = self.intf.hal_handle
        req_spec.l2segment_key_or_handle.l2segment_handle = self.seg.hal_handle
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
            if GlobalOptions.rerun == False:
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

def __delete(objs, reqmsg_class, config_method):
    return __hal_api_handler(objs, reqmsg_class, config_method,\
                            "PrepareHALDeleteRequestSpec", "ProcessHALDeleteResponse")

def init():
    global HalChannel
    if IsHalDisabled(): return

    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    cfglogger.info("Creating GRPC channel to HAL")
    server = 'localhost:' + port
    HalChannel = grpc.insecure_channel(server)
    cfglogger.info("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(HalChannel).result()
    return

def IsHalDisabled():
    return GlobalOptions.no_hal or GlobalOptions.dryrun

def ConfigureLifs(objlist):
    if IsHalDisabled(): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    __config(objlist, interface_pb2.LifRequestMsg,
             stub.LifCreate)
    return

def ConfigureInterfaces(objlist, update = False):
    if IsHalDisabled(): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    api = stub.InterfaceCreate
    if update: api = stub.InterfaceUpdate
    __config(objlist, interface_pb2.InterfaceRequestMsg, api)
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
    stub = tenant_pb2.TenantStub(HalChannel)
    __config(objlist, tenant_pb2.TenantRequestMsg,
             stub.TenantCreate)
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
    __config(objlist, l2segment_pb2.L2SegmentRequestMsg,
             stub.L2SegmentCreate)
    return

def ConfigureEndpoints(objlist):
    if IsHalDisabled(): return
    stub = endpoint_pb2.EndpointStub(HalChannel)
    __config(objlist, endpoint_pb2.EndpointRequestMsg,
             stub.EndpointCreate)
    return

def ConfigureSessions(objlist):
    if IsHalDisabled(): return
    stub = session_pb2.SessionStub(HalChannel)
    __config(objlist, session_pb2.SessionRequestMsg,
             stub.SessionCreate)
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
    if update: api = stub.SecurityProfileUpdate
    __config(objlist, nwsec_pb2.SecurityProfileRequestMsg, api)
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

def ConfigureNetworks(objlist):
    if IsHalDisabled(): return
    stub = nw_pb2.NetworkStub(HalChannel)
    __config(objlist, nw_pb2.NetworkRequestMsg,
             stub.NetworkCreate)
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
    __config(objlist, wring_pb2.WRingRequestMsg,
             stub.WRingGetMeta)
    return

def ConfigureAcls(objlist):
    if IsHalDisabled(): return
    stub = acl_pb2.AclStub(HalChannel)
    __config(objlist, acl_pb2.AclRequestMsg,
             stub.AclCreate)
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

def ConfigureCqs(objlist):
    if IsHalDisabled(): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaCqRequestMsg,
             stub.RdmaCqCreate)
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

