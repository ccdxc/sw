import interface_pb2
import random
import math
import os
import types_pb2
import kh_pb2
import utils
import config_mgr

if (utils.mbt_v2()):
    from mbt_v2.msg import GrpcReqRspMsg
else:
    from grpc_meta.msg import GrpcReqRspMsg

cpu_if_type_max = 1
cpu_if_type_seen = 0
port_num = 1
num_uplink_ifs = 0
max_uplink_ifs = 31
if_id = [i for i in range(128,255)]
uplink_if_id = []
g_lif_names = []

def PreCreateCb(data, req_spec, resp_spec):
    global g_lif_names
    req_spec.request[0].key_or_handle.interface_id = random.choice(if_id)
    if_id.remove(req_spec.request[0].key_or_handle.interface_id)

    if (utils.mbt_v2()):
        ext_refs = {}

        # Ignore NONE type
        if req_spec.request[0].type == interface_pb2.IF_TYPE_NONE:
            return False

    if req_spec.request[0].HasField("if_enic_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
    elif req_spec.request[0].HasField("if_uplink_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_UPLINK
        uplink_if_id.append(req_spec.request[0].key_or_handle.interface_id)
    elif req_spec.request[0].HasField("if_uplink_pc_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_UPLINK_PC
    elif req_spec.request[0].HasField("if_tunnel_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_TUNNEL
    elif req_spec.request[0].HasField("if_cpu_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_CPU
    elif req_spec.request[0].HasField("if_app_redir_info"):
        req_spec.request[0].type = interface_pb2.IF_TYPE_APP_REDIR

    if req_spec.request[0].type == interface_pb2.IF_TYPE_CPU:
        req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC

        if (utils.mbt_v2()):
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info, ext_refs=ext_refs)
        else:
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_TUNNEL:
        if (len(g_lif_names) == 0):
            create_lif()
        req_spec.request[0].if_tunnel_info.encap_type = interface_pb2.IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS
        if (utils.mbt_v2()):
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_tunnel_info.prop_mpls_info, ext_refs=ext_refs)
        else:
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_tunnel_info.prop_mpls_info)
        if req_spec.request[0].if_tunnel_info.prop_mpls_info.substrate_ip.ip_af == types_pb2.IP_AF_INET6:
            GrpcReqRspMsg.generate_ip_address(req_spec.request[0].if_tunnel_info.prop_mpls_info.substrate_ip, types_pb2.IP_AF_INET)
        for overlay_ip in req_spec.request[0].if_tunnel_info.prop_mpls_info.overlay_ip:
            GrpcReqRspMsg.generate_ip_address(overlay_ip, types_pb2.IP_AF_INET)
        if req_spec.request[0].if_tunnel_info.prop_mpls_info.tunnel_dest_ip.ip_af == types_pb2.IP_AF_INET6:
            GrpcReqRspMsg.generate_ip_address(req_spec.request[0].if_tunnel_info.prop_mpls_info.tunnel_dest_ip, types_pb2.IP_AF_INET)
        req_spec.request[0].if_tunnel_info.prop_mpls_info.source_gw.prefix.ipv4_subnet.prefix_len = GrpcReqRspMsg.generate_ip_address(req_spec.request[0].if_tunnel_info.prop_mpls_info.source_gw.prefix.ipv4_subnet.address, types_pb2.IP_AF_INET)
        name = random.choice(g_lif_names)
        req_spec.request[0].if_tunnel_info.prop_mpls_info.lif_name = name[1]

    if req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK_PC or \
       req_spec.request[0].type == interface_pb2.IF_TYPE_UPLINK:
        global num_uplink_ifs
        global max_uplink_ifs
        num_uplink_ifs += 1
        if num_uplink_ifs > max_uplink_ifs:
            req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC

            if (utils.mbt_v2()):
                GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info, ext_refs=ext_refs)
            else:
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
        if req_spec.request[0].if_enic_info.HasField("classic_enic_info"):
            req_spec.request[0].if_enic_info.ClearField("classic_enic_info")
            if (utils.mbt_v2()):
                GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info.enic_info, ext_refs=ext_refs)
            else:
                GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info.enic_info)

        req_spec.request[0].if_enic_info.enic_type = random.choice([interface_pb2.IF_ENIC_TYPE_USEG, \
           interface_pb2.IF_ENIC_TYPE_PVLAN, interface_pb2.IF_ENIC_TYPE_DIRECT])
        print (req_spec.request[0].if_enic_info.enic_info.mac_address)
        req_spec.request[0].if_enic_info.enic_info.mac_address &= 0xFFFFFF00FFFF
        print (req_spec.request[0].if_enic_info.enic_info.mac_address)
        req_spec.request[0].if_enic_info.enic_info.mac_address |= \
            (req_spec.request[0].if_enic_info.pinned_uplink_if_key_handle.interface_id << 16)
        print (req_spec.request[0].if_enic_info.enic_info.mac_address)

    if (utils.mbt_v2()):
        return True

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def create_and_get_l2seg_key():
    if (utils.mbt_v2()):
        constraints = None
        ext_refs = {}
        return utils.create_config_from_kh('L2SegmentKeyHandle', constraints, ext_refs)
    else:
        l2seg_key_type = getattr(kh_pb2, 'L2SegmentKeyHandle')
        return config_mgr.CreateConfigFromKeyType(l2seg_key_type)

def create_lif():
    if (utils.mbt_v2()):
        constraints = None
        ext_refs = {}
        utils.create_config_from_kh('LifKeyHandle', constraints, ext_refs)
    else:
        key_type = getattr(kh_pb2, 'LifKeyHandle')
        lif_object = config_mgr.GetExtRefObjectFromKey(config_mgr.CreateConfigFromKeyType(key_type))
        msg = lif_object._msg_cache[config_mgr.ConfigObjectMeta.CREATE]
        lif_name = [msg.request[0].key_or_handle.lif_id, msg.request[0].name] 
        g_lif_names.append(lif_name)
    return

def PreUpdateCb(data, req_spec, resp_spec):
    if (utils.mbt_v2()):
        ext_refs = {}

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
            if (utils.mbt_v2()):
                GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info, ext_refs=ext_refs)
            else:
                GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_enic_info)

    if req_spec.request[0].type == interface_pb2.IF_TYPE_TUNNEL:
        req_spec.request[0].if_tunnel_info.encap_type = interface_pb2.IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS
        if (utils.mbt_v2()):
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_tunnel_info.prop_mpls_info, ext_refs=ext_refs)
        else:
            GrpcReqRspMsg.static_generate_message(req_spec.request[0].if_tunnel_info.prop_mpls_info)
        if req_spec.request[0].if_tunnel_info.prop_mpls_info.substrate_ip.ip_af == types_pb2.IP_AF_INET6:
            GrpcReqRspMsg.generate_ip_address(req_spec.request[0].if_tunnel_info.prop_mpls_info.substrate_ip, types_pb2.IP_AF_INET)
        if req_spec.request[0].if_tunnel_info.prop_mpls_info.overlay_ip[0].ip_af == types_pb2.IP_AF_INET6:
            GrpcReqRspMsg.generate_ip_address(req_spec.request[0].if_tunnel_info.prop_mpls_info.overlay_ip[0], types_pb2.IP_AF_INET)
        if req_spec.request[0].if_tunnel_info.prop_mpls_info.tunnel_dest_ip.ip_af == types_pb2.IP_AF_INET6:
            GrpcReqRspMsg.generate_ip_address(req_spec.request[0].if_tunnel_info.prop_mpls_info.tunnel_dest_ip, types_pb2.IP_AF_INET)
        req_spec.request[0].if_tunnel_info.prop_mpls_info.source_gw.prefix.ipv4_subnet.prefix_len = GrpcReqRspMsg.generate_ip_address(req_spec.request[0].if_tunnel_info.prop_mpls_info.source_gw.prefix.ipv4_subnet.address, types_pb2.IP_AF_INET)
 
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
        req_spec.request[0].if_enic_info.classic_enic_info.native_l2segment_id = 0
        # Classic Enic's with the same L2Segments are not allowed.
        # So create a new object.
        l2seg_key = create_and_get_l2seg_key()
        req_spec.request[0].if_enic_info.classic_enic_info.l2segment_key_handle.extend([l2seg_key])
        req_spec.request[0].if_enic_info.pinned_uplink_if_key_handle.if_handle = 0


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
    req_spec.request[0].rss.type = 0
    req_spec.request[0].hw_lif_id = 0

def LifPostCreateCb(data, req_spec, resp_spec):
    global g_lif_names
    print("Reached Post Create CB")
    lif_name = [req_spec.request[0].key_or_handle.lif_id, req_spec.request[0].name]
    g_lif_names.append(lif_name)

def LifPostDeleteCb(data, req_spec, resp_spec):
    global g_lif_names
    for i in range(len(g_lif_names)):
        if g_lif_names[i][0] == req_spec.request[0].key_or_handle.lif_id:
            del g_lif_names[i]
            break

def PostDeleteCb(data, req_spec, resp_spec):
    if_id.append(req_spec.request[0].key_or_handle.interface_id)
    if uplink_if_id.count(req_spec.request[0].key_or_handle.interface_id):
        uplink_if_id.remove(req_spec.request[0].key_or_handle.interface_id)
