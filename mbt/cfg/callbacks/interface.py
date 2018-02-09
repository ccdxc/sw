import interface_pb2
import random
import math
from grpc_meta.msg import GrpcReqRspMsg
import os
import types_pb2
import config_mgr
import kh_pb2

cpu_if_type_max = 1
cpu_if_type_seen = 0
port_num = 1
num_uplink_ifs = 0
max_uplink_ifs = 31

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

    if req_spec.request[0].type == interface_pb2.IF_TYPE_CPU:
        global cpu_if_type_max
        global cpu_if_type_seen
        if cpu_if_type_seen < cpu_if_type_max:
            cpu_if_type_seen += 1
        else:
            req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_TUNNEL:
        req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
        GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK_PC or \
       req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK:
        global num_uplink_ifs
        global max_uplink_ifs
        num_uplink_ifs += 1
        if num_uplink_ifs > max_uplink_ifs:
            req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK:
        global port_num
        req_spec.request[0].if_uplink_info.port_num = port_num
        port_num += 1
        if port_num >= 8:
            port_num = 1
        req_spec.request[0].if_uplink_info.native_l2segment_id = 0
    elif req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK_PC:
        req_spec.request[0].if_uplink_pc_info.native_l2segment_id = 0
    elif req_spec.request[0].type == interface_pb2.IF_TYPE_ENIC:
        req_spec.request[0].if_enic_info.enic_type = interface_pb2.IF_ENIC_TYPE_CLASSIC
        req_spec.request[0].if_enic_info.classic_enic_info.native_l2segment_handle = 0
        # Classic Enic's with the same L2Segments are not allowed.
        # So create a new object.
        l2seg_key = create_and_get_l2seg_key()
        req_spec.request[0].if_enic_info.classic_enic_info.l2segment_key_handle.extend([l2seg_key])
        req_spec.request[0].if_enic_info.pinned_uplink_if_handle = 0

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def create_and_get_l2seg_key():
    l2seg_key_type = getattr(kh_pb2, 'L2SegmentKeyHandle')
    return config_mgr.CreateConfigFromKeyType(l2seg_key_type)
    

def PreUpdateCb(data, req_spec, resp_spec):
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

    if req_spec.request[0].type == interface_pb2.IF_TYPE_CPU:
        global cpu_if_type_max
        global cpu_if_type_seen
        if cpu_if_type_seen < cpu_if_type_max:
            cpu_if_type_seen += 1
        else:
            req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_TUNNEL:
        req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
        GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK:
        global port_num
        req_spec.request[0].if_uplink_info.port_num = port_num
        port_num += 1
        if port_num >= 8:
            port_num = 1
        req_spec.request[0].if_uplink_info.native_l2segment_id = 0
    elif req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK_PC:
        req_spec.request[0].if_uplink_pc_info.native_l2segment_id = 0
    elif req_spec.request[0].type == interface_pb2.IF_TYPE_ENIC:
        req_spec.request[0].if_enic_info.enic_type = interface_pb2.IF_ENIC_TYPE_CLASSIC
        req_spec.request[0].if_enic_info.classic_enic_info.native_l2segment_handle = 0
        # Classic Enic's with the same L2Segments are not allowed.
        # So create a new object.
        l2seg_key = create_and_get_l2seg_key()
        req_spec.request[0].if_enic_info.classic_enic_info.l2segment_key_handle.extend([l2seg_key])
        req_spec.request[0].if_enic_info.pinned_uplink_if_handle = 0


def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)

def LifPreCreateCb(data, req_spec, resp_spec):
    type_num = random.randint(0, 7)
    req_spec.request[0].ClearField('lif_qstate_map')
    req_spec.request[0].ClearField('tx_qos_class')
    req_spec.request[0].ClearField('rx_qos_class')
    req_spec.request[0].enable_rdma = False
    req_spec.request[0].packet_filter.receive_promiscuous = False
    req_spec.request[0].packet_filter.receive_broadcast = False
    req_spec.request[0].packet_filter.receive_all_multicast = False

