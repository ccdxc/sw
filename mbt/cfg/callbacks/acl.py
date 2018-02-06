import acl_pb2
import random
import types_pb2

def PreCreateCb(data, req_spec, resp_spec):
    if req_spec.request[0].match.HasField("ip_selector"):
        if req_spec.request[0].match.ip_selector.HasField("udp_selector"):
            req_spec.request[0].match.ip_selector.udp_selector.src_port_range.port_low = 1
            req_spec.request[0].match.ip_selector.udp_selector.src_port_range.port_high = 65535
            req_spec.request[0].match.ip_selector.udp_selector.dst_port_range.port_low = 1
            req_spec.request[0].match.ip_selector.udp_selector.dst_port_range.port_high = 65535
        elif req_spec.request[0].match.ip_selector.HasField("tcp_selector"):
            req_spec.request[0].match.ip_selector.tcp_selector.src_port_range.port_low = 1
            req_spec.request[0].match.ip_selector.tcp_selector.src_port_range.port_high = 65535
            req_spec.request[0].match.ip_selector.tcp_selector.dst_port_range.port_low = 1
            req_spec.request[0].match.ip_selector.tcp_selector.dst_port_range.port_high = 65535

        if req_spec.request[0].match.ip_selector.ip_af == types_pb2.IP_AF_NONE:
            req_spec.request[0].match.ip_selector.src_prefix.address.ip_af = types_pb2.IP_AF_NONE
            req_spec.request[0].match.ip_selector.dst_prefix.address.ip_af = types_pb2.IP_AF_NONE
        elif req_spec.request[0].match.ip_selector.ip_af == types_pb2.IP_AF_INET:
            req_spec.request[0].match.ip_selector.src_prefix.address.ip_af = types_pb2.IP_AF_INET
            req_spec.request[0].match.ip_selector.dst_prefix.address.ip_af = types_pb2.IP_AF_INET
            req_spec.request[0].match.ip_selector.src_prefix.address.v4_addr = random.randint(0, 99999)
        elif req_spec.request[0].match.ip_selector.ip_af == types_pb2.IP_AF_INET6:
            req_spec.request[0].match.ip_selector.src_prefix.address.ip_af = types_pb2.IP_AF_INET6
            req_spec.request[0].match.ip_selector.dst_prefix.address.ip_af = types_pb2.IP_AF_INET6
            req_spec.request[0].match.ip_selector.src_prefix.address.v6_addr = random.getrandbits(128).to_bytes(16, byteorder='big')

    if req_spec.request[0].action.action != acl_pb2.ACL_ACTION_REDIRECT:
        req_spec.request[0].action.ClearField("redirect_if_key_handle")

    req_spec.request[0].action.ClearField("internal_actions")

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    PreCreateCb(data, req_spec, resp_spec)

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
