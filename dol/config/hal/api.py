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
import tcp_proxy_cb_pb2            as tcpcb_pb2

import endpoint_pb2_grpc    as endpoint_pb2_grpc
import l2segment_pb2_grpc   as l2segment_pb2_grpc
import tenant_pb2_grpc      as tenant_pb2_grpc
import interface_pb2_grpc   as interface_pb2_grpc
import session_pb2_grpc     as session_pb2_grpc
import nwsec_pb2_grpc       as nwsec_pb2_grpc
import nw_pb2_grpc          as nw_pb2_grpc
import telemetry_pb2_grpc   as telemetry_pb2_grpc

HAL_MAX_BATCH_SIZE = 16
import tcp_proxy_cb_pb2_grpc       as tcpcb_pb2_grpc

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

def __process_response(resp_msg, req_msg, req_objs):
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
        req_obj.ProcessHALResponse(req_spec, resp_spec)
        if resp_spec.api_status != types_pb2.API_STATUS_OK:
            assert(0)
    return


def __config(objs, reqmsg_class, config_method):
    req_msg = reqmsg_class()
    req_objs = []
    count = 0
    for obj in objs:
        req_spec = req_msg.request.add()
        obj.PrepareHALRequestSpec(req_spec)
        req_objs.append(obj)

        count += 1
        if count >= HAL_MAX_BATCH_SIZE:
            resp_msg = config_method(req_msg)
            __process_response(resp_msg, req_msg, req_objs)
            req_msg = reqmsg_class()
            req_objs = []
            count = 0

    if count != 0:
        resp_msg = config_method(req_msg)
        __process_response(resp_msg, req_msg, req_objs)
    return

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
    return

def IsHalDisabled():
    return GlobalOptions.no_hal or GlobalOptions.dryrun

def ConfigureLifs(objlist):
    if IsHalDisabled(): return
    stub = interface_pb2_grpc.InterfaceStub(HalChannel)
    __config(objlist, interface_pb2.LifRequestMsg,
             stub.LifCreate)
    return

def ConfigureInterfaces(objlist, update = False):
    if IsHalDisabled(): return
    stub = interface_pb2_grpc.InterfaceStub(HalChannel)
    api = stub.InterfaceCreate
    if update: api = stub.InterfaceUpdate
    __config(objlist, interface_pb2.InterfaceRequestMsg, api)
    return

def ConfigureInterfaceSegmentAssociations(intfs, segs):
    if IsHalDisabled(): return
    stub = interface_pb2_grpc.InterfaceStub(HalChannel)
    
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
    stub = tenant_pb2_grpc.TenantStub(HalChannel)
    __config(objlist, tenant_pb2.TenantRequestMsg,
             stub.TenantCreate)
    return

def ConfigureTcpCbs(objlist):
    if IsHalDisabled(): return
    stub = tcpcb_pb2_grpc.TcpCbStub(HalChannel)
    __config(objlist, tcpcb_pb2.TcpCbRequestMsg,
             stub.TcpCbCreate)
    return

def ConfigureSegments(objlist):
    if IsHalDisabled(): return
    stub = l2segment_pb2_grpc.L2SegmentStub(HalChannel)
    __config(objlist, l2segment_pb2.L2SegmentRequestMsg,
             stub.L2SegmentCreate)
    return

def ConfigureEndpoints(objlist):
    if IsHalDisabled(): return
    stub = endpoint_pb2_grpc.EndpointStub(HalChannel)
    __config(objlist, endpoint_pb2.EndpointRequestMsg,
             stub.EndpointCreate)
    return

def ConfigureSessions(objlist):
    if IsHalDisabled(): return
    stub = session_pb2_grpc.SessionStub(HalChannel)
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
    stub = nwsec_pb2_grpc.NwSecurityStub(HalChannel)
    api = stub.SecurityProfileCreate
    if update: api = stub.SecurityProfileUpdate
    __config(objlist, nwsec_pb2.SecurityProfileRequestMsg, api)
    return

def ConfigureSpanSessions(objlist):
    if IsHalDisabled(): return
    stub = telemetry_pb2_grpc.TelemetryStub(HalChannel)
    __config(objlist, telemetry_pb2.MirrorSessionConfigMsg,
             stub.MirrorSessionCreate)
    return

def ConfigureNetworks(objlist):
    if IsHalDisabled(): return
    stub = nw_pb2_grpc.NetworkStub(HalChannel)
    __config(objlist, nw_pb2.NetworkRequestMsg,
             stub.NetworkCreate)
    return
