#! /usr/bin/python3

import os
import sys
import pdb
import types_pb2
import infra.common.defs    as defs
import iris.config.hal.defs      as haldefs

import grpc

from infra.common.glopts  import GlobalOptions
from infra.common.logging import logger
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
import acl_pb2              as acl_pb2
import qos_pb2              as qos_pb2
import proxy_pb2            as proxy_pb2
import l4lb_pb2             as l4lb_pb2
import rdma_pb2             as rdma_pb2
import cpucb_pb2            as cpucb_pb2
import multicast_pb2        as multicast_pb2
import system_pb2           as system_pb2
import dos_pb2              as dos_pb2
import nic_pb2              as nic_pb2
import internal_pb2         as internal_pb2
import nvme_pb2             as nvme_pb2
if GlobalOptions.gft:
    import gft_pb2              as gft_pb2

HAL_MAX_BATCH_SIZE = 64

HalChannel = None
def __process_response(resp_msg, req_msg, req_objs, respcb):
    if req_msg is None or req_msg.DESCRIPTOR.name == "SystemGetRequest":
        for idx in range(len(req_objs)):
            req_obj = req_objs[idx]
            resp_spec = resp_msg
            getattr(req_obj, respcb)(None, resp_spec)
            if resp_spec.api_status != types_pb2.API_STATUS_OK:
                logger.error(" HAL Returned API Status:%d" % (resp_spec.api_status))
                assert(0)
        return

    if (req_msg.DESCRIPTOR.fields_by_name["request"].label == 3):
        num_req_specs = len(req_msg.request)
        num_resp_specs = len(resp_msg.response)

        if num_req_specs != num_resp_specs:
            logger.error(" - Bad # of resp_specs:%d Expected:%d" %\
                        (num_resp_specs, num_req_specs))
            assert(0)

        for idx in range(len(req_msg.request)):
            req_spec = req_msg.request[idx]
            resp_spec = resp_msg.response[idx]
            req_obj = req_objs[idx]
            getattr(req_obj, respcb)(req_spec, resp_spec)
            if resp_spec.api_status == types_pb2.API_STATUS_EXISTS_ALREADY:
                logger.info(" Object exists already")
            elif resp_spec.api_status != types_pb2.API_STATUS_OK:
                logger.error(" HAL Returned API Status:%d" % (resp_spec.api_status))
                assert(0)
    else:
        req_obj = req_objs[0]
        getattr(req_obj, respcb)(req_msg.request, resp_msg.response)
        if resp_msg.response.api_status == types_pb2.API_STATUS_EXISTS_ALREADY:
            logger.info(" Object exists already")
        elif resp_msg.response.api_status != types_pb2.API_STATUS_OK:
            logger.error(" HAL Returned API Status:%d" % (resp_msg.response.api_status))
            assert(0)

    return

def __invoke_api(api, req_msg):
    if req_msg:
        resp_msg = api(req_msg)
    else:
        resp_msg = api(types_pb2.Empty())
    return resp_msg

def __hal_api_handler(objs, reqmsg_class, api, reqcb, respcb):
    req_msg = None
    if reqmsg_class:
        req_msg = reqmsg_class()
    req_objs = []
    count = 0
    for obj in objs:
        req_spec = None
        if req_msg:
            if (req_msg.DESCRIPTOR.fields_by_name["request"].label == 3):
                req_spec = req_msg.request.add()
                getattr(obj,reqcb)(req_spec)
            elif (req_msg.DESCRIPTOR.name == "SystemGetRequest"):
                getattr(obj,reqcb)(req_msg)
            else:
                getattr(obj,reqcb)(req_msg.request)
        req_objs.append(obj)
        count += 1
        if count >= HAL_MAX_BATCH_SIZE:
            resp_msg = __invoke_api(api, req_msg)
            if req_msg and req_msg.DESCRIPTOR.name == "SessionGetRequestMsg":
                for resp in resp_msg:
                    __process_response(resp, req_msg, req_objs, respcb)
            else:
                __process_response(resp_msg, req_msg, req_objs, respcb)
            req_msg = reqmsg_class()
            req_objs = []
            count = 0

    if count != 0:
        resp_msg = __invoke_api(api, req_msg)
        if req_msg and req_msg.DESCRIPTOR.name == "SessionGetRequestMsg":
            for resp in resp_msg:
                __process_response(resp, req_msg, req_objs, respcb)
        else:
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

def IsHalDisabled():
    return GlobalOptions.no_hal or GlobalOptions.dryrun
def IsObjectListInFeatureSet(objs):
    return GlobalOptions.feature_set in objs[0].meta.feature_set
def IsConfigAllowed(objs):
    assert(GlobalOptions.agent is False)
    if not len(objs): return False
    if not IsObjectListInFeatureSet(objs):
        logger.info("Skipping config: Object not in feature set")
        return False
    if IsHalDisabled(): return False
    return True

def init():
    global HalChannel
    if IsHalDisabled(): return

    if 'MBT_GRPC_PORT' in os.environ: # If MBT toggle mode is enabled
        # assert GlobalOptions.mbt
        port = os.environ['MBT_GRPC_PORT']
    elif 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    logger.info("Creating GRPC channel to HAL on port %s" %(port))
    server = 'localhost:' + port
    if 'HAL_GRPC_IP' in os.environ:
        server = os.environ['HAL_GRPC_IP'] + ':' + port
    HalChannel = grpc.insecure_channel(server)
    logger.info("HAL Server IP and Port = ", server)
    logger.info("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(HalChannel).result()
    logger.info("Connected to HAL!")
    if GlobalOptions.mbt:
        SignalingClient.Connect()
        logger.info("Connected to the Model based tester")
    return

def ConfigureLifs(objlist, update = False):
    if not IsConfigAllowed(objlist): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    api = stub.LifUpdate if update else stub.LifCreate
    msg = interface_pb2.LifRequestMsg
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetLifs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    __get(objlist, interface_pb2.LifGetRequestMsg,
          stub.LifGet)
    return

def ConfigureInterfaces(objlist, update = False):
    if not IsConfigAllowed(objlist): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    msg = interface_pb2.InterfaceRequestMsg
    api = stub.InterfaceUpdate if update else stub.InterfaceCreate
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetInterfaces(objlist):
    if not IsConfigAllowed(objlist):
        return
    stub = interface_pb2.InterfaceStub(HalChannel)
    __get(objlist, interface_pb2.InterfaceGetRequestMsg,
          stub.InterfaceGet)
    return

def ConfigureDevice(objlist, update = False):
    if not IsConfigAllowed(objlist): return
    stub = nic_pb2.NicStub(HalChannel)
    msg = nic_pb2.DeviceRequestMsg
    api = stub.DeviceUpdate if update else stub.DeviceCreate
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetDevices(objlist):
    if not IsConfigAllowed(objlist):
        return
    stub = nic_pb2.NicStub(HalChannel)
    __get(objlist, nic_pb2.DeviceGetRequestMsg,
          stub.DeviceGet)
    return

def ConfigureInterfaceSegmentAssociations(objlist):
    if not IsConfigAllowed(objlist): return
    stub = interface_pb2.InterfaceStub(HalChannel)
    __config(objlist, interface_pb2.InterfaceL2SegmentRequestMsg,
             stub.AddL2SegmentOnUplink)
    return

def ConfigureTenants(objlist):
    if not IsConfigAllowed(objlist): return
    stub = vrf_pb2.VrfStub(HalChannel)
    msg = vrf_pb2.VrfRequestMsg
    __config(objlist, msg,
             stub.VrfCreate)
    if GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetTenants(objlist):
    if not IsConfigAllowed(objlist):
        return
    stub = vrf_pb2.VrfStub(HalChannel)
    __get(objlist, vrf_pb2.VrfGetRequestMsg,
          stub.VrfGet)
    return

def ConfigureTcpCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.TcpCbRequestMsg,
             stub.TcpCbCreate)
    return

def ConfigureIpsecCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.IpsecCbRequestMsg,
             stub.IpsecCbCreate)
    return

def UpdateTcpCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.TcpCbRequestMsg,
             stub.TcpCbUpdate)
    return

def UpdateTlsCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.TlsCbRequestMsg,
             stub.TlsCbUpdate)
    return

def UpdateIpsecCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.IpsecCbRequestMsg,
             stub.IpsecCbUpdate)
    return

def ConfigureCpuCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = cpucb_pb2.CpuCbStub(HalChannel)
    __config(objlist, cpucb_pb2.CpuCbRequestMsg,
             stub.CpuCbCreate)
    return

def UpdateCpuCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = cpucb_pb2.CpuCbStub(HalChannel)
    __config(objlist, cpucb_pb2.CpuCbRequestMsg,
             stub.CpuCbUpdate)
    return

def GetCpuCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = cpucb_pb2.CpuCbStub(HalChannel)
    __config(objlist, cpucb_pb2.CpuCbGetRequestMsg,
             stub.CpuCbGet)
    return

def ConfigureRawrCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.RawrCbRequestMsg,
             stub.RawrCbCreate)
    return

def UpdateRawrCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.RawrCbRequestMsg,
             stub.RawrCbUpdate)
    return

def GetRawrCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.RawrCbGetRequestMsg,
             stub.RawrCbGet)
    return

def ConfigureProxyrCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.ProxyrCbRequestMsg,
             stub.ProxyrCbCreate)
    return

def UpdateProxyrCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.ProxyrCbRequestMsg,
             stub.ProxyrCbUpdate)
    return

def GetProxyrCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.ProxyrCbGetRequestMsg,
             stub.ProxyrCbGet)
    return

def ConfigureRawcCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.RawcCbRequestMsg,
             stub.RawcCbCreate)
    return

def UpdateRawcCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.RawcCbRequestMsg,
             stub.RawcCbUpdate)
    return

def GetRawcCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.RawcCbGetRequestMsg,
             stub.RawcCbGet)
    return

def ConfigureProxycCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.ProxycCbRequestMsg,
             stub.ProxycCbCreate)
    return

def UpdateProxycCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.InternalRequestMsg,
             stub.ProxycCbUpdate)
    return

def GetProxycCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.ProxycCbGetRequestMsg,
             stub.ProxycCbGet)
    return

def GetTcpCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.TcpCbGetRequestMsg,
             stub.TcpCbGet)
    return

def GetIpsecCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.IpsecCbGetRequestMsg,
             stub.IpsecCbGet)
    return

def GetTlsCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.TlsCbGetRequestMsg,
             stub.TlsCbGet)
    return

def ConfigureProxyService(objlist):
    if not IsConfigAllowed(objlist): return
    stub = proxy_pb2.ProxyStub(HalChannel)
    __config(objlist, proxy_pb2.ProxyRequestMsg,
             stub.ProxyEnable)
    return

def ConfigureTlsCbs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.TlsCbRequestMsg,
             stub.TlsCbCreate)
    return

def ConfigureSegments(objlist):
    if not IsConfigAllowed(objlist): return
    stub = l2segment_pb2.L2SegmentStub(HalChannel)
    msg = l2segment_pb2.L2SegmentRequestMsg
    __config(objlist, msg, stub.L2SegmentCreate)
    if GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetSegments(objlist):
    if not IsConfigAllowed(objlist): return
    stub = l2segment_pb2.L2SegmentStub(HalChannel)
    __get(objlist, l2segment_pb2.L2SegmentGetRequestMsg,
          stub.L2SegmentGet)
    return

def ConfigureEndpoints(objlist):
    if not IsConfigAllowed(objlist): return
    stub = endpoint_pb2.EndpointStub(HalChannel)
    __config(objlist, endpoint_pb2.EndpointRequestMsg,
             stub.EndpointCreate)
    return

def GetEndpoints(objlist):
    if not IsConfigAllowed(objlist): return
    stub = endpoint_pb2.EndpointStub(HalChannel)
    __get(objlist, endpoint_pb2.EndpointGetRequestMsg,
          stub.EndpointGet)
    return

def ConfigureSessions(objlist):
    if not IsConfigAllowed(objlist): return
    stub = session_pb2.SessionStub(HalChannel)
    __config(objlist, session_pb2.SessionRequestMsg,
             stub.SessionCreate)
    return

def GetSessions(objlist):
    if not IsConfigAllowed(objlist): return
    stub = session_pb2.SessionStub(HalChannel)
    __get(objlist, session_pb2.SessionGetRequestMsg,
          stub.SessionGet)
    return

def ConfigureLifsForObjects(objlist):
    lif_objlist = []
    for obj in objlist:
        lif_objlist.append(obj.lif)
    if not IsConfigAllowed(lif_objlist): return
    logger.info("Configuring %d LIFs." % len(lif_objlist))
    ConfigureLifs(lif_objlist)
    return

def ConfigureSecurityProfiles(objlist, update = False):
    if not IsConfigAllowed(objlist): return
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
    if not IsConfigAllowed(objlist): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    __get(objlist, nwsec_pb2.SecurityProfileGetRequestMsg,
          stub.SecurityProfileGet)
    return

def ConfigureSecurityGroups(objlist, update = False):
    if not IsConfigAllowed(objlist): return
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
    if not IsConfigAllowed(objlist): return
    stub = dos_pb2.DosStub(HalChannel)
    api = stub.DoSPolicyCreate
    if update: api = stub.DoSPolicyUpdate
    __config(objlist, dos_pb2.DoSPolicyRequestMsg, api)
    return

def GetSecurityGroups(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    __get(objlist, nwsec_pb2.SecurityGroupGetRequestMsg,
          stub.SecurityGroupGet)

def ConfigureSecurityGroupPolicies(objlist, update = False):
    if not IsConfigAllowed(objlist): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    api = stub.SecurityPolicyCreate
    msg = nwsec_pb2.SecurityPolicyRequestMsg
    if update: api = stub.SecurityPolicyUpdate
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetSecurityGroupPolicies(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nwsec_pb2.NwSecurityStub(HalChannel)
    __get(objlist, nwsec_pb2.SecurityGroupPolicyGetRequestMsg,
          stub.SecurityGroupPolicyGet)
    return

def ConfigureSpanSessions(objlist):
    if not IsConfigAllowed(objlist): return
    stub = telemetry_pb2.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.MirrorSessionRequestMsg,
             stub.MirrorSessionCreate)
    return

def DeleteSpanSessions(objlist):
    if not IsConfigAllowed(objlist): return
    stub = telemetry_pb2.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.MirrorSessionDeleteRequestMsg,
             stub.MirrorSessionDelete)
    return

def ConfigureCollectors(objlist):
    if not IsConfigAllowed(objlist): return
    stub = telemetry_pb2.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.CollectorRequestMsg,
             stub.CollectorCreate)
    return

def ConfigureNetworks(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nw_pb2.NetworkStub(HalChannel)
    msg = nw_pb2.NetworkRequestMsg
    __config(objlist, msg, stub.NetworkCreate)
    if GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def GetNetworks(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nw_pb2.NetworkStub(HalChannel)
    __get(objlist, nw_pb2.NetworkGetRequestMsg,
          stub.NetworkGet)
    return

def GetDscrAolObjectState(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.DescrAolRequestMsg,
             stub.DescrAolGet)
    return

def GetRingEntries(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.WRingGetEntriesRequestMsg,
             stub.WRingGetEntries)
    return

def GetRingMeta(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __get(objlist, internal_pb2.WRingRequestMsg,
             stub.WRingGetMeta)
    return

def SetRingMeta(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.WRingRequestMsg,
             stub.WRingSetMeta)
    return

def ConfigureAcls(objlist, update = False):
    if not IsConfigAllowed(objlist): return
    stub = acl_pb2.AclStub(HalChannel)
    api = stub.AclUpdate if update else stub.AclCreate
    msg = acl_pb2.AclRequestMsg
    __config(objlist, msg, api)
    if not update and GlobalOptions.mbt:
        SignalingClient.SendSignalingData(msg.__name__)
        SignalingClient.Wait()
    return

def DeleteAcls(objlist):
    if not IsConfigAllowed(objlist): return
    stub = acl_pb2.AclStub(HalChannel)
    __delete(objlist, acl_pb2.AclDeleteRequestMsg,
             stub.AclDelete)
    return


def ConfigureL4LbBackends(objlist):
    if not IsConfigAllowed(objlist): return
    stub = l4lb_pb2.L4LbStub(HalChannel)
    __config(objlist, l4lb_pb2.L4LbBackendRequestMsg,
             stub.L4LbBackendCreate)
    return

def ConfigureL4LbServices(objlist):
    if not IsConfigAllowed(objlist): return
    stub = l4lb_pb2.L4LbStub(HalChannel)
    __config(objlist, l4lb_pb2.L4LbServiceRequestMsg,
             stub.L4LbServiceCreate)
    return

def GetCryptoAsymKey(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.CryptoApiRequestMsg,
             stub.CryptoApiInvoke)
    return

def GetCryptoCert(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.CryptoApiRequestMsg,
             stub.CryptoApiInvoke)
    return

def UpdateCryptoCerts(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.CryptoApiRequestMsg,
             stub.CryptoApiInvoke)

def GetCryptoKey(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.CryptoKeyCreateRequestMsg,
             stub.CryptoKeyCreate)
    return

def UpdateCryptoKeys(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.CryptoKeyUpdateRequestMsg,
             stub.CryptoKeyUpdate)
    return

def ConfigureAqs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaAqRequestMsg,
             stub.RdmaAqCreate)
    return

def ConfigureQps(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaQpRequestMsg,
             stub.RdmaQpCreate)
    return

def ModifyQps(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaQpUpdateRequestMsg,
             stub.RdmaQpUpdate)

def ConfigureAhs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaAhRequestMsg,
             stub.RdmaAhCreate)
    return

def ConfigureCqs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaCqRequestMsg,
             stub.RdmaCqCreate)
    return

def ConfigureEqs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaEqRequestMsg,
             stub.RdmaEqCreate)
    return

def ConfigureMrs(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaMemRegRequestMsg,
             stub.RdmaMemReg)
    return

def ConfigureKeys(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaAllocLkeyRequestMsg,
             stub.RdmaAllocLkey)
    return

def ConfigureMws(objlist):
    if not IsConfigAllowed(objlist): return
    stub = rdma_pb2.RdmaStub(HalChannel)
    __config(objlist, rdma_pb2.RdmaMemWindowRequestMsg,
             stub.RdmaMemWindow)
    return

def ConfigureProxyCbService(objlist):
    if not IsConfigAllowed(objlist): return
    stub = proxy_pb2.ProxyStub(HalChannel)
    __config(objlist, proxy_pb2.ProxyFlowConfigRequestMsg,
             stub.ProxyFlowConfig)
    return

def GetQidProxycbGetFlowInfo(objlist):
    if not IsConfigAllowed(objlist): return
    stub = proxy_pb2.ProxyStub(HalChannel)
    __config(objlist, proxy_pb2.ProxyGetFlowInfoRequestMsg,
             stub.ProxyGetFlowInfo)
    return

def ConfigureMulticastGroups(objlist, update = False):
    if not IsConfigAllowed(objlist): return
    stub = multicast_pb2.MulticastStub(HalChannel)
    api =  stub.MulticastEntryUpdate if update else stub.MulticastEntryCreate
    __config(objlist, multicast_pb2.MulticastEntryRequestMsg, api)
    return

def GetBarcoRingEntries(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __config(objlist, internal_pb2.BarcoGetReqDescrEntryRequestMsg,
             stub.BarcoGetReqDescrEntry)
    return

def GetBarcoRingMeta(objlist):
    if not IsConfigAllowed(objlist): return
    stub = internal_pb2.InternalStub(HalChannel)
    __get(objlist, internal_pb2.BarcoGetRingMetaRequestMsg,
             stub.BarcoGetRingMeta)
    return

def ConfigureQosClass(objlist):
    if not IsConfigAllowed(objlist): return
    stub = qos_pb2.QOSStub(HalChannel)
    __config(objlist, qos_pb2.QosClassRequestMsg,
             stub.QosClassCreate)
    return

def ConfigureGftExmProfiles(objlist):
    if not IsConfigAllowed(objlist): return
    if GlobalOptions.gft:
        stub = gft_pb2.GftStub(HalChannel)
        __config(objlist, gft_pb2.GftExactMatchProfileRequestMsg,
                 stub.GftExactMatchProfileCreate)
    return

def ConfigureGftFlows(objlist):
    if not IsConfigAllowed(objlist): return
    if GlobalOptions.gft:
        stub = gft_pb2.GftStub(HalChannel)
        __config(objlist, gft_pb2.GftExactMatchFlowEntryRequestMsg,
                 stub.GftExactMatchFlowEntryCreate)
    return

def GetSystem(objlist):
    if not IsConfigAllowed(objlist): return
    stub = system_pb2.SystemStub(HalChannel)
    __get(objlist, system_pb2.SystemGetRequest, stub.SystemGet)
    return

def ConfigureDropMonitorRules(objlist):
    if not IsConfigAllowed(objlist): return
    stub = telemetry_pb2.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.DropMonitorRuleRequestMsg,
             stub.DropMonitorRuleCreate)
    return

def NvmeEnable(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nvme_pb2.NvmeStub(HalChannel)
    __config(objlist, nvme_pb2.NvmeEnableRequestMsg,
             stub.NvmeEnable)
    return

def NvmeSqCreate(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nvme_pb2.NvmeStub(HalChannel)
    __config(objlist, nvme_pb2.NvmeSqRequestMsg,
             stub.NvmeSqCreate)
    return

def NvmeCqCreate(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nvme_pb2.NvmeStub(HalChannel)
    __config(objlist, nvme_pb2.NvmeCqRequestMsg,
             stub.NvmeCqCreate)
    return

def NvmeNsCreate(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nvme_pb2.NvmeStub(HalChannel)
    __config(objlist, nvme_pb2.NvmeNsRequestMsg,
             stub.NvmeNsCreate)
    return

def NvmeSessionCreate(objlist):
    if not IsConfigAllowed(objlist): return
    stub = nvme_pb2.NvmeStub(HalChannel)
    __config(objlist, nvme_pb2.NvmeSessRequestMsg,
             stub.NvmeSessCreate)
    return
