import types_pb2
from grpc_meta.msg import GrpcReqRspMsg
import cfg.callbacks.endpoint as endpoint
import random

def CollectorPreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    global max_infra_types
    global infra_vrf_id
    global customer_vrf_id

    if req_spec.request[0].src_ip.ip_af == types_pb2.IP_AF_INET6:
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].src_ip, types_pb2.IP_AF_INET)

    while len(endpoint.ep_vrf_ids) < 1:
        endpoint.CreateEndpoint()

    req_spec.request[0].vrf_key_handle.vrf_id = random.choice(endpoint.ep_vrf_ids)
    idx = endpoint.ep_vrf_ids.index(req_spec.request[0].vrf_key_handle.vrf_id)
    req_spec.request[0].dest_ip.ip_af = types_pb2.IP_AF_INET
    req_spec.request[0].dest_ip.v4_addr = endpoint.ep_ipv4_addrs[idx]

    req_spec.request[0].encap.encap_type = types_pb2.ENCAP_TYPE_DOT1Q

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
