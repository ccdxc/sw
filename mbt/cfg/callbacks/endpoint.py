import types_pb2
from grpc_meta.msg import GrpcReqRspMsg

ep_ipv4_addrs = []
ep_vrf_ids = []

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    global max_infra_types
    global infra_vrf_id
    global customer_vrf_id

    if req_spec.request[0].key_or_handle.endpoint_key.HasField("l3_key"):
        GrpcReqRspMsg.static_generate_message(req_spec.request[0].key_or_handle.endpoint_key.l2_key)

    if req_spec.request[0].endpoint_attrs.ip_address[0].WhichOneof("v4_or_v6") == "v4_addr":
        req_spec.request[0].endpoint_attrs.ip_address[0].ip_af = types_pb2.IP_AF_INET
    else:
        req_spec.request[0].endpoint_attrs.ip_address[0].ip_af = types_pb2.IP_AF_INET6

def PostCreateCb(data, req_spec, resp_spec):
    global ep_ipv4_addrs
    global ep_vrf_ids

    if req_spec.request[0].endpoint_attrs.ip_address[0].ip_af == types_pb2.IP_AF_INET:
        ep_ipv4_addrs.append(req_spec.request[0].endpoint_attrs.ip_address[0].v4_addr)
        ep_vrf_ids.append(req_spec.request[0].vrf_key_handle.vrf_id)
