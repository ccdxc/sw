import types_pb2
import telemetry_pb2
from grpc_meta.msg import GrpcReqRspMsg
import cfg.callbacks.endpoint as endpoint
import cfg.callbacks.l2segment as l2segment
import random

ip_protocol = [types_pb2.IPPROTO_ICMP, types_pb2.IPPROTO_TCP, types_pb2.IPPROTO_UDP, types_pb2.IPPROTO_ESP]
collector_count = 0

def CollectorPreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    global max_infra_types
    global infra_vrf_id
    global customer_vrf_id
    global l2seg_dot1q
    global collector_count
    
    if collector_count > 7:
        return False

    if req_spec.request[0].src_ip.ip_af == types_pb2.IP_AF_INET6:
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].src_ip, types_pb2.IP_AF_INET)

    while len(endpoint.ep_vrf_ids) < 1:
        endpoint.CreateEndpoint()

    req_spec.request[0].vrf_key_handle.vrf_id = random.choice(endpoint.ep_vrf_ids)
    idx = endpoint.ep_vrf_ids.index(req_spec.request[0].vrf_key_handle.vrf_id)
    req_spec.request[0].dest_ip.ip_af = types_pb2.IP_AF_INET
    req_spec.request[0].dest_ip.v4_addr = endpoint.ep_ipv4_addrs[idx]

    req_spec.request[0].encap.encap_type = types_pb2.ENCAP_TYPE_DOT1Q
    req_spec.request[0].format = telemetry_pb2.IPFIX
    if len(l2segment.l2seg_dot1q) != 0:
        req_spec.request[0].l2seg_key_handle = l2segment.l2seg_dot1q[0]
    collector_count = collector_count + 1
    return True

def FlowMatchPreCreateCb(data, req_spec, resp_spec):
    if req_spec.request[0].match.src_address[0].address.WhichOneof("Address") == "range":
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].match.src_address[0].address.range.ipv4_range.low_ipaddr, types_pb2.IP_AF_INET)
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].match.src_address[0].address.range.ipv4_range.high_ipaddr, types_pb2.IP_AF_INET)
        if (req_spec.request[0].match.src_address[0].address.range.ipv4_range.low_ipaddr.v4_addr >
            req_spec.request[0].match.src_address[0].address.range.ipv4_range.high_ipaddr.v4_addr):
            tmp = req_spec.request[0].match.src_address[0].address.range.ipv4_range.high_ipaddr.v4_addr
            req_spec.request[0].match.src_address[0].address.range.ipv4_range.high_ipaddr.v4_addr = req_spec.request[0].match.src_address[0].address.range.ipv4_range.low_ipaddr.v4_addr
            req_spec.request[0].match.src_address[0].address.range.ipv4_range.low_ipaddr.v4_addr = tmp
    else:
        req_spec.request[0].match.src_address[0].address.prefix.ipv4_subnet.prefix_len = GrpcReqRspMsg.generate_ip_address(req_spec.request[0].match.src_address[0].address.prefix.ipv4_subnet.address, types_pb2.IP_AF_INET)

    if req_spec.request[0].match.dst_address[0].address.WhichOneof("Address") == "range":
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].match.dst_address[0].address.range.ipv4_range.low_ipaddr, types_pb2.IP_AF_INET)
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].match.dst_address[0].address.range.ipv4_range.high_ipaddr, types_pb2.IP_AF_INET)
        if (req_spec.request[0].match.dst_address[0].address.range.ipv4_range.low_ipaddr.v4_addr >
            req_spec.request[0].match.dst_address[0].address.range.ipv4_range.high_ipaddr.v4_addr):
            tmp = req_spec.request[0].match.dst_address[0].address.range.ipv4_range.high_ipaddr.v4_addr
            req_spec.request[0].match.dst_address[0].address.range.ipv4_range.high_ipaddr.v4_addr = req_spec.request[0].match.dst_address[0].address.range.ipv4_range.low_ipaddr.v4_addr
            req_spec.request[0].match.dst_address[0].address.range.ipv4_range.low_ipaddr.v4_addr = tmp
    else:
        req_spec.request[0].match.dst_address[0].address.prefix.ipv4_subnet.prefix_len = GrpcReqRspMsg.generate_ip_address(req_spec.request[0].match.dst_address[0].address.prefix.ipv4_subnet.address, types_pb2.IP_AF_INET)
    req_spec.request[0].action.agg_scheme[0] = telemetry_pb2.NONE
    req_spec.request[0].action.action[0] = telemetry_pb2.MIRROR
    req_spec.request[0].match.protocol = random.choice(ip_protocol)
    req_spec.request[0].match.ether_type = 0x0800
