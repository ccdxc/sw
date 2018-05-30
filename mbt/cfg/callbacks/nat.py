import types_pb2
from grpc_meta.msg import GrpcReqRspMsg
import utils
import config_mgr

def PreCreateCb(data, req_spec, resp_spec):
    if req_spec.request[0].address[0].WhichOneof("Address") == "range":
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].address[0].range.ipv4_range.low_ipaddr, types_pb2.IP_AF_INET)
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].address[0].range.ipv4_range.high_ipaddr, types_pb2.IP_AF_INET)
        if (req_spec.request[0].address[0].range.ipv4_range.low_ipaddr.v4_addr >
            req_spec.request[0].address[0].range.ipv4_range.high_ipaddr.v4_addr):
            tmp = req_spec.request[0].address[0].range.ipv4_range.high_ipaddr.v4_addr
            req_spec.request[0].address[0].range.ipv4_range.high_ipaddr.v4_addr = req_spec.request[0].address[0].range.ipv4_range.low_ipaddr.v4_addr
            req_spec.request[0].address[0].range.ipv4_range.low_ipaddr.v4_addr = tmp
    else:
        req_spec.request[0].address[0].prefix.ipv4_subnet.prefix_len = GrpcReqRspMsg.generate_ip_address(req_spec.request[0].address[0].prefix.ipv4_subnet.address, types_pb2.IP_AF_INET)

    if req_spec.request[0].port_range.port_low > req_spec.request[0].port_range.port_high:
        tmp_port = req_spec.request[0].port_range.port_high
        req_spec.request[0].port_range.port_high = req_spec.request[0].port_range.port_low
        req_spec.request[0].port_range.port_low = tmp_port

def PrefixLentoNetmask(prefix_len):
    if prefix_len == 32:
        return 0xFFFFFFFF
    else:
        return (((1 << prefix_len) - 1) << (32 - prefix_len))

def MappingPreCreateCb(data, req_spec, resp_spec):
    if (utils.mbt_v2()):
        cache_create_msg = utils.get_create_req_msg_from_kh(req_spec.request[0].nat_pool)
    else:
        nat_pool_obj = config_mgr.GetExtRefObjectFromKey(req_spec.request[0].nat_pool)
        cache_create_msg = nat_pool_obj._msg_cache[config_mgr.ConfigObjectMeta.CREATE]

    ip_low = 0
    ip_hi = 0
    if cache_create_msg.request[0].address[0].HasField("range"):
        ip_low = cache_create_msg.request[0].address[0].range.ipv4_range.low_ipaddr.v4_addr
        ip_hi = cache_create_msg.request[0].address[0].range.ipv4_range.high_ipaddr.v4_addr
    else:
        subnet = cache_create_msg.request[0].address[0].prefix.ipv4_subnet.address.v4_addr
        prefix_len = cache_create_msg.request[0].address[0].prefix.ipv4_subnet.prefix_len
        ip_low = (subnet & PrefixLentoNetmask(prefix_len))
        ip_hi = ip_low + (1 << (32 - prefix_len)) - 1

    req_spec.request[0].key_or_handle.svc.port = 0
    if req_spec.request[0].key_or_handle.svc.ip_addr.ip_af == types_pb2.IP_AF_INET6:
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].key_or_handle.svc.ip_addr, types_pb2.IP_AF_INET)

    while ((req_spec.request[0].key_or_handle.svc.ip_addr.v4_addr >= ip_low) and
        (req_spec.request[0].key_or_handle.svc.ip_addr.v4_addr <= ip_hi)):
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].key_or_handle.svc.ip_addr, types_pb2.IP_AF_INET)
