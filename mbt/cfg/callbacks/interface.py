import interface_pb2
import random
import math
from grpc_meta.msg import GrpcReqRspMsg
import os
import types_pb2


def PreCreateCb(data, req_spec, resp_spec):
    if req_spec.request[0].HasField("if_enic_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
    elif req_spec.request[0].HasField("if_uplink_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_UPLINK
    elif req_spec.request[0].HasField("if_uplink_pc_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_UPLINK_PC
    elif req_spec.request[0].HasField("if_tunnel_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_TUNNEL
    elif req_spec.request[0].HasField("if_cpu_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_CPU
    elif req_spec.request[0].HasField("if_app_redir_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_APP_REDIR

    if req_spec.request[0].type == interface_pb2.IF_TYPE_TUNNEL:
        req_spec.request[0].type = interface_pb2.IF_TYPE_UPLINK
        GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_uplink_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK:
        req_spec.request[0].if_uplink_info.port_num = 1
        req_spec.request[0].if_uplink_info.native_l2segment_id = 0
    elif req_spec.request[0].type == interface_pb2.IF_TYPE_ENIC:
        req_spec.request[0].if_enic_info.enic_type = interface_pb2.IF_ENIC_TYPE_CLASSIC
        req_spec.request[0].if_enic_info.classic_enic_info.native_l2segment_handle = 0
        req_spec.request[0].if_enic_info.pinned_uplink_if_handle = 0

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    PreCreateCb(data, req_spec, resp_spec)

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)

def LifPreCreateCb(data, req_spec, resp_spec):
    type_num = random.randint(0, 7)
    req_spec.request[0].ClearField('lif_qstate_map')
    req_spec.request[0].enable_rdma = False
    req_spec.request[0].packet_filter.receive_promiscuous = False
    req_spec.request[0].packet_filter.receive_broadcast = False
    req_spec.request[0].packet_filter.receive_all_multicast = False
