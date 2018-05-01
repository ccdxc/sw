import types_pb2
from grpc_meta.msg import GrpcReqRspMsg

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

def MappingPreCreateCb(data, req_spec, resp_spec):
    req_spec.request[0].key_or_handle.svc.port = 0
    if req_spec.request[0].key_or_handle.svc.ip_addr.ip_af == types_pb2.IP_AF_INET6:
        GrpcReqRspMsg.generate_ip_address(req_spec.request[0].key_or_handle.svc.ip_addr, types_pb2.IP_AF_INET)
